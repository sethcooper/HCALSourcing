#include "RecoTBCalo/HcalTBObjectUnpacker/interface/HcalTBSourcePositionDataUnpacker.h"
#include "FWCore/Utilities/interface/Exception.h"
#include <iostream>
#include <string>
#include <map>

using namespace std;

/// Structure for Source Position Data
// same as slow data format
struct xdaqSourcePositionDataFormat {
  uint32_t cdfHeader[4];
  uint16_t n_doubles;
  uint16_t n_strings;
  uint16_t key_length;
  uint16_t string_value_length;
  char     start_of_data; // see below
  // char[n_doubles*key_length] doubles names
  // double[n_doubles] doubles values
  // char[n_strings*key_length] strings names
  // char[n_strings*string_value_length] strings values
  // xdaqCommonDataFormatTrailer
};


namespace hcaltb {
  
  void HcalTBSourcePositionDataUnpacker::unpack(const FEDRawData&  raw,
						HcalSourcePositionData&    hspd) {

    if (raw.size()<3*8) {
      throw cms::Exception("Missing Data") << "No data in the source position data block";
    }

    const struct xdaqSourcePositionDataFormat* sp =
      (const struct xdaqSourcePositionDataFormat*)(raw.data());

    if (raw.size()<sizeof(xdaqSourcePositionDataFormat)) {
      throw cms::Exception("DataFormatError","Fragment too small");
    }

    map<string,double> sp_dblmap;
    map<string,string> sp_strmap;

    #ifdef DEBUG
    cout << "#doubles = "   << sp->n_doubles << endl;;
    cout << "#strings = "   << sp->n_strings << endl;
    cout << "key_length = " << sp->key_length << endl;
    cout << "string_value_length = " << sp->string_value_length << endl;
    #endif

    // List of doubles:
    const char   *keyptr = &sp->start_of_data;
    const double *valptr =
      (const double *)(&sp->start_of_data + sp->n_doubles*sp->key_length);

    for (int i=0; i<sp->n_doubles; i++) {
      //#ifdef DEBUG
      cout << keyptr << " = " << *valptr << endl;
      //#endif
      sp_dblmap[keyptr] = *valptr;
      keyptr += sp->key_length;
      valptr++;
    }

    // List of strings:
    keyptr = (const char *)valptr;
    const char *strptr = (keyptr + sp->n_strings*sp->key_length);

    for (int i=0; i<sp->n_strings; i++) {
      //#ifdef DEBUG
      cout << keyptr << " = " << strptr << endl;
      //#endif
      sp_strmap[keyptr] = strptr;
      keyptr += sp->key_length;
      strptr += sp->string_value_length;
    }

    //cout << "reel = " << sp_dblmap["REEL"] << endl;
    //cout << "index = " << sp_dblmap["INDEX"] << endl;
    //cout << "motor current = " << sp_dblmap["MOTOR_CURRENT"] << endl;
    //cout << "speed = " << sp_dblmap["SPEED"] << endl;
    //cout << "message counter = " << sp_dblmap["MESSAGE"] << endl;
    // timestamp1
    // timestamp2

    // Now fill the input objects:
    //TODO remove second timestamp
    //TODO these are all ints in the object; should be doubles?
    hspd.set(sp_dblmap["MESSAGE"], //double message_counter
		 sp_dblmap["TIME_STAMP1"],//double timestamp1_sec
		 sp_dblmap["TIME_STAMP2"],//double timestamp1_usec
		 -1,//double timestamp2_sec
		 -1,//double timestamp2_usec
		 -1,//double status
		 sp_dblmap["INDEX"],//double index_counter
		 sp_dblmap["REEL"],//double reel_counter
		 sp_dblmap["MOTOR_CURRENT"],//double motor_current
		 sp_dblmap["MOTOR_VOLTAGE"],//double motor_voltage
		 -1,//double tube_id
		 -1,//double driver_id
     -1,//double source_id
     sp_strmap["CURRENT_TUBENAME_FROM_COORD"],
     "", // current tubeName from SD
     sp_strmap["LAST_COMMAND"],
     sp_strmap["MESSAGE"]
     );
  }

}

