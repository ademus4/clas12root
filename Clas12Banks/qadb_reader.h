#ifndef QADB_READER_H
#define QADB_READER_H

#ifdef CLAS_QADB
#include "QADB.h"
#endif

#include <iostream>
#include <string>
#include <vector>

namespace clas12 {
  using std::string;
  
  class qadb_reader {

  public:
    qadb_reader(int runNb=0);
    //virtual ~qadb_reader()=default;  



  public:
    bool isValid();
    
    void addQARequirement(string req){_reqsQA.push_back(req);_masksAdded=false;};
    void setQARequirements( std::vector<string> reqs){_reqsQA=reqs;_masksAdded=false;};
    
    void requireOkForAsymmetry(bool ok){_reqOKAsymmetry=ok;};
    void requireGolden(bool ok){_reqGolden=ok;};
    bool passQAReqs(int evNb);

    void setRun(int runNb){_runNb=runNb;}

    void copySettings(const qadb_reader& other);
    
    
    
  private:
    
    qadb_reader()=default;

    
    std::vector<string> _reqsQA;//!
    bool _reqOKAsymmetry{false};//!
    bool _reqGolden{false};//!
    int _runNb{0};//!
    bool _masksAdded{false};

    //ifdefs must go last , or can lead to issues with PROOF
    //i.e. refences are slighty shifted
  
#ifdef CLAS_QADB
  public:
    
    int GetMask(){return _qa.GetMask();};
    bool query(int runNb, int evNb){return _qa.Query(runNb,evNb);};
    bool querybyFileNb(int runNb, int fileNb){return _qa.QueryByFilenum(runNb,fileNb);};
    int getMaxFileNb(int runNb){return _qa.GetMaxFilenum(runNb);};
    int getFileNb(){return _qa.GetFilenum();};
    int getRunNb(){return _qa.GetRunnum();};
    bool isGolden(int runNb, int evNb){return _qa.Golden(runNb,evNb);};
    int getMinEventNb(){return _qa.GetEvnumMin();};
    int getMaxEventNb(){return _qa.GetEvnumMax();};
    int getDefect(int sector=0){return _qa.GetDefect(sector);};
    bool hasDefect(const char * defectName, int sector){return _qa.HasDefect(defectName,sector);};
    bool hasDefect(const char * defectName){return _qa.HasDefect(defectName);};
    string getComment(){return _qa.GetComment();};
    bool isOkForAsymmetry(int runNb, int evNb){return _qa.OkForAsymmetry(runNb,evNb);};

    double getCharge(){return _qa.GetCharge();};
    void resetAccCharge(){_qa.ResetAccumulatedCharge();};
    void addMask(const char * defectName, bool maskBit){_qa.SetMaskBit(defectName,maskBit);};
    void addAllMasks();
    double getAccCharge(){return _qa.GetAccumulatedCharge();};

  private: 
    QADB _qa;//!  

#else
  public:
    double getAccCharge(){return 0.0;};

#endif


  
  };

}
#endif /* QADB_READER_H */
