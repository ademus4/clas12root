/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "clas12reader.h"

namespace clas12 {

  clas12reader::clas12reader(std::string filename,std::vector<long> tags){
    cout<<" clas12reader::clas12reader reading "<<filename.data()<<endl;
    _reader.setTags(tags);
    _reader.open(filename.data()); //keep a pointer to the reader

    // hipo::dictionary  factory;
    _reader.readDictionary(_factory);

    //initialise banks pointers
    if(_factory.hasSchema("RECFT::Particle"))_bftbparts = std::make_shared<ftbparticle>(_factory.getSchema("RECFT::Particle"));
    if(_factory.hasSchema("REC::Particle"))_bparts = std::make_shared<particle>(_factory.getSchema("REC::Particle"),_bftbparts);
    if(_factory.hasSchema("MC::Lund"))_bmcparts = std::make_shared<mcparticle>(_factory.getSchema("MC::Lund"));
    if(_factory.hasSchema("REC::CovMat"))_bcovmat= std::make_shared<covmatrix>(_factory.getSchema("REC::CovMat"));
    if(_factory.hasSchema("RECFT::Event"))_bftbevent  = std::make_shared<clas12::ftbevent>(_factory.getSchema("RECFT::Event"));
    if(_factory.hasSchema("RUN::config"))_brunconfig  = std::make_shared<clas12::runconfig>(_factory.getSchema("RUN::config"));
    if(_factory.hasSchema("REC::Event"))_bevent  = std::make_shared<clas12::event>(_factory.getSchema("REC::Event"),_bftbevent);
    if(_factory.hasSchema("REC::Calorimeter"))_bcal   = std::make_shared<calorimeter>(_factory.getSchema("REC::Calorimeter"));
    if(_factory.hasSchema("REC::Scintillator"))_bscint = std::make_shared<scintillator>(_factory.getSchema("REC::Scintillator"));
    if(_factory.hasSchema("REC::Track"))_btrck  = std::make_shared<tracker>(_factory.getSchema("REC::Track"));
    if(_factory.hasSchema("REC::Traj"))_btraj  = std::make_shared<traj>(_factory.getSchema("REC::Traj"));
    if(_factory.hasSchema("REC::Cherenkov"))_bcher  = std::make_shared<cherenkov>(_factory.getSchema("REC::Cherenkov"));
    if(_factory.hasSchema("REC::ForwardTagger"))_bft    = std::make_shared<forwardtagger>(_factory.getSchema("REC::ForwardTagger"));
    if(_factory.hasSchema("HEL::online"))_bhelonline  = std::make_shared<clas12::helonline>(_factory.getSchema("HEL::online"));
    if(_factory.hasSchema("HEL::flip"))_bhelflip  = std::make_shared<clas12::helflip>(_factory.getSchema("HEL::flip"),_brunconfig);
    //if(_factory.hasSchema("RAW::vtp"))_bvtp    = std::make_shared<clas12::vtp>(_factory.getSchema("RAW::vtp"));
    //if(_factory.hasSchema("RAW::scaler"))_bscal = std::make_shared<clas12::scaler>(_factory.getSchema("RAW::scaler"));
 
  }
  ///////////////////////////////////////////////////////
  ///read the data
  void clas12reader::clearEvent(){
    //clear previous event
    _n_rfdets=0;
    _n_rcdets=0;
    _n_rfts=0;
    
    _detParticles.clear();
    _pids.clear();
 
    _isRead=false;
  }

  ///////////////////////////////////////////////////////////////
  ///This function may be called before readEvent to allow checking
  ///of Pids by external routines
   const std::vector<short>& clas12reader::preCheckPids(){
    //regular particle bank
     if(_isRead) return _pids; //already read return current pids
     hipoRead();
    _event.getStructure(*_bparts.get());
    if(_bftbparts.get())_event.getStructure(*_bftbparts.get());//FT based PID particle bank
   
    //First check if event passes criteria
    _nparts=_bparts->getRows();
    _pids.clear();
    _pids.reserve(_nparts);
 
    
    //Loop over particles and find their Pid
    for(ushort i=0;i<_nparts;i++){
      if(!_useFTBased){
	_bparts->setEntry(i);
	_pids.emplace_back(_bparts->getPid());
      }
      else{
	if(_bftbparts->getRows()){
	  _bftbparts->setEntry(i);
	  _pids.emplace_back(_bftbparts->getPid());
	}
	else{//if not ftbased use FD based
	  _bparts->setEntry(i);
	  _pids.emplace_back(_bparts->getPid());
	}
      }
	
    }
    return _pids;
  }
  bool clas12reader::readEvent(){
    //get pid of tracks and save in _pids
    preCheckPids();
   
     //check if event is of the right type
    if(!passPidSelect()){
      _pids.clear(); //reset so read next event in preChekPids
      return false;
    }
    //Special run banks
    if(_brunconfig.get())_event.getStructure(*_brunconfig.get());
   
    //now getthe data for the rest of the banks
    if(_bmcparts.get())_event.getStructure(*_bmcparts.get());
    if(_bcovmat.get())_event.getStructure(*_bcovmat.get());
    if(_bevent.get())_event.getStructure(*_bevent.get());
    if(_bftbevent.get())_event.getStructure(*_bftbevent.get());
    if(_bcal.get())_event.getStructure(*_bcal.get());
    if(_bscint.get())_event.getStructure(*_bscint.get());
    if(_btrck.get())_event.getStructure(*_btrck.get());
    if(_btraj.get())_event.getStructure(*_btraj.get());
    if(_bcher.get())_event.getStructure(*_bcher.get());
    if(_bft.get())_event.getStructure(*_bft.get());
    //if(_bvtp.get())_event.getStructure(*_bvtp.get());
    //if(_bscal.get())_event.getStructure(*_bscal.get());

    for(auto ibank:_addBanks){//if any additional banks requested get those
      _event.getStructure(*ibank.get());
    }
    
    return true;
  }
  ////////////////////////////////////////////////////////
  ///initialise next event from the reader
  bool clas12reader::next(){

    //keep going until we get an event that passes
    bool validEvent=false;
    while(_reader.next()){
      clearEvent();
      _nevent++;
      if(readEvent()){ //got one
	validEvent=true;
	break;
      }
    }
    if(!validEvent) return false;//no more events in reader
    //can proceed with valid event
    sort();
  
    return true;
  }
  ////////////////////////////////////////////////////////
  ///initialise next event from the reader
  bool clas12reader::nextInRecord(){
     
    //keep going until we get an event that passes
    bool validEvent=false;
    while(_reader.nextInRecord()){
      clearEvent();
      _nevent++;
      if(readEvent()){ //got one
	validEvent=true;
	break;
      }
    }
    if(!validEvent) return false;//no more events in record

    //can proceed with valid event
    sort();
 
    return true;
  }
  ////////////////////////////////////////////////////////
  /// Loop over particles and find their region
  /// Add appropriate region_partcle to event particle vector
  void clas12reader::sort(){

    if(_nparts==0) return;
   
    _n_rfdets=0;
    _n_rcdets=0;
    _n_rfts=0;

    _detParticles.clear();
    _detParticles.reserve(_nparts);

     
    //Loop over particles and find their region
    for(ushort i=0;i<_nparts;i++){ 
      _bparts->setEntry(i);
      
      //Check if FDet particle
      if(_rfdets.empty()) addARegionFDet();
      if(_rfdets[_n_rfdets]->sort()){
	//	add a FDet particle to the event list
	_detParticles.emplace_back(_rfdets[_n_rfdets]);
	_n_rfdets++;
	//check if need more vector entries
	//only required of previous events have
	//less particles than this
	if(_n_rfdets==_rfdets.size())
	  addARegionFDet();
	continue;
      }
      
      //Check if CDet particle
      if(_rcdets.empty()) addARegionCDet();
      if(_rcdets[_n_rcdets]->sort()){
	//	add a FDet particle to the event list
	_detParticles.emplace_back(_rcdets[_n_rcdets]);
	_n_rcdets++;
	//check if need more vector entries
	//only required of previous events have
	//less particles than this
	if(_n_rcdets==_rcdets.size())
	  addARegionCDet();
	continue;
      }
	 
       //Check if FT particle
      if(_rfts.empty())addARegionFT();
      if(_rfts[_n_rfts]->sort()){
	//add a FDet particle to the event list
	_detParticles.emplace_back(_rfts[_n_rfts]);
	_n_rfts++;
	//check if need more vector entries
	//only required of previous events have
	//less particles than this
	if(_n_rfts==_rfts.size())
	  addARegionFT();
	continue;
      }
    }
 
 
  }
  bool clas12reader::passPidSelect(){
    //if no selections take event
    if(_pidSelect.empty()&&_pidSelectExact.empty()) return true;

    //check is there is at least enough particles
    if(_pidSelectExact.size()+_pidSelect.size()>_nparts)
      return false;

    //check if any unwanted particles
    if(_zeroOfRestPid){
      auto uniquePids=_pids;//make a copy
      std::sort(uniquePids.begin(), uniquePids.end());
      auto ip = std::unique(uniquePids.begin(), uniquePids.begin() + _nparts); 
      uniquePids.resize(std::distance(uniquePids.begin(), ip));
      //now just loop over the unique particle types
      for(auto const& pid : uniquePids){
	//check if we have a PID not given in a selection
	if(!(std::count(_givenPids.begin(),_givenPids.end(), pid)))
	  return false;
      }
    }
  
    //check for requested exact matches
    for(auto const& select : _pidSelectExact){
      if(!(select.second==getNPid(select.first)))
	return false;
    }
    
    //check for requeseted at least  matches
    for(auto const& select : _pidSelect){
      if((select.second>getNPid(select.first)))
	return false;
    }
    return true;
  }
  
   ////////////////////////////////////////////////////////
  ///Filter and return detParticles by given PID
  std::vector<region_part_ptr> clas12reader::getByID(int id){
    return container_filter(_detParticles, [id](region_part_ptr dr)
			    {return dr->getPid()==id;});
  }
  std::vector<region_part_ptr> clas12reader::getByRegion(int ir){
    return container_filter(_detParticles, [ir](region_part_ptr dr)
			    {return dr->getRegion()==ir;});
  }
  std::vector<region_part_ptr> clas12reader::getByCharge(int ch){
    return container_filter(_detParticles, [ch](region_part_ptr dr)
			    {return dr->par()->getCharge()==ch;});
  }

}
