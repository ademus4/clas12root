/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "ftbevent.h"

namespace clas12 {

   
  ftbevent::ftbevent(hipo::schema __schema): hipo::bank(__schema) {
    auto sch=getSchema();
     
    if(sch.exists("category"))_cat_order  = sch.getEntryOrder("category");
    else if(sch.exists("EvCAT")){
      std::cout<<"Warning, using deprecated item name EvCAT instead of category for REC::FTEvent"<<std::endl;
      _cat_order  = sch.getEntryOrder("EvCAT");
    }
    _st_order  = sch.getEntryOrder("startTime");
  }
}
