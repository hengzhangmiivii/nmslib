/**
 * Non-metric Space Library
 *
 * Authors: Bilegsaikhan Naidan (https://github.com/bileg), Leonid Boytsov (http://boytsov.info).
 * With contributions from Lawrence Cayton (http://lcayton.com/).
 *
 * For the complete list of contributors and further details see:
 * https://github.com/searchivarius/NonMetricSpaceLib 
 * 
 * Copyright (c) 2010--2013
 *
 * This code is released under the
 * Apache License Version 2.0 http://www.apache.org/licenses/.
 *
 */
#ifndef PARAMS_H
#define PARAMS_H

#include <string>
#include <vector>
#include <limits>
#include <map>
#include <set>

#include "lcstrategy.h"
#include "logging.h"

namespace similarity {

using std::string;
using std::vector;
using std::multimap;
using std::set;


class AnyParams {
public:
  /* 
   * Each element of the MethodDesc array is in the form:
   * <param name>=<param value>
   */
  AnyParams(const vector<string>& MethodDesc) :ParamNames(0), ParamValues(0) {
    set<string> seen;
    for (unsigned i = 0; i < MethodDesc.size(); ++i) {
      vector<string>  OneParamPair;
      if (!SplitStr(MethodDesc[i], OneParamPair, '=') ||
          OneParamPair.size() != 2) {
        LOG(FATAL) << "Wrong format of the method argument: '" << MethodDesc[i] << "' should be in the format: <Name>=<Value>";
      }
      const string& Name = OneParamPair[0];
      const string& sVal = OneParamPair[1];

      if (seen.count(Name)) {
        LOG(FATAL) << "Duplicate parameter: " << Name;
      }
      seen.insert(Name);

      ParamNames.push_back(Name);
      ParamValues.push_back(sVal);
    }
  }

  template <typename ParamType> 
  void ChangeParam(const string& Name, const ParamType& Value) {
    for (unsigned i = 0; i < ParamNames.size(); ++i) 
    if (Name == ParamNames[i]) {
      stringstream str;

      str << Value;
      ParamValues[i] = str.str();
      return;
    }
    LOG(FATAL) << "Parameter not found: " << Name;
  } 

  AnyParams(){}
  AnyParams(const vector<string>& Names, const vector<string>& Values) 
            : ParamNames(Names), ParamValues(Values) {}

  vector<string>  ParamNames;
  vector<string>  ParamValues;

};

class AnyParamManager {
public:
  AnyParamManager(const AnyParams& params) : params(params), seen() {
    if (params.ParamNames.size() != params.ParamValues.size()) {
      LOG(FATAL) << "Bug: different # of parameters and values";
    }
  }

  template <typename ParamType>
  void GetParamRequired(const string&  Name, ParamType& Value) {
    GetParam<ParamType>(Name, Value, true);
  }

  /*
   * The default Value should be set before calling this function.
   * If the parameter not specified, the Value remains unchanged.
   * For instance:
   *    AnyParams ParamManager(SomeParams);
   *    int val = 3;
   *    ParamManager.GetParamOptional("name", val);  
   *     // if name wasn't not present in SomeParams, val remains equal to 3.
   */
  template <typename ParamType>
  void GetParamOptional(const string&  Name, ParamType& Value) {
    GetParam<ParamType>(Name, Value, false);
  }

  /*
   * Takes a list of exceptions and extracts all parameter values, 
   * except parameters from this list. The extracted parameters
   * are added to the list of parameters already seen.
   */
  AnyParams ExtractParametersExcept(const vector<string>& ExceptList) {
    set<string> except(ExceptList.begin(), ExceptList.end());

    vector<string> names, values;

    for (size_t i = 0; i < params.ParamNames.size(); ++i) {
      const string& name = params.ParamNames[i];
      if (except.count(name) == 0) { // Not on the exception list
        names.push_back(name);
        values.push_back(params.ParamValues[i]);
        seen.insert(name);
      }
    }

    return AnyParams(names, values);
  }

  ~AnyParamManager() {
    bool bFail = false;
    for (const auto Name: params.ParamNames) 
    if (!seen.count(Name)) {
      LOG(ERROR) << "Unknown parameter: " << Name;
      bFail = true;
    }
    if (bFail) LOG(FATAL) << "Unknown parameters found, aborting!";
  }
private:
  const AnyParams&  params;
  set<string>       seen;

  template <typename ParamType>
  void GetParam(const string&  Name, ParamType& Value, bool bRequired) {
    bool bFound = false;
    /* 
     * This loop is reasonably efficient, unless
     * we have thousands of parameters (which will not happen)
     */
    for (size_t i =0; i < params.ParamNames.size(); ++i) 
    if (Name == params.ParamNames[i]) {
      bFound = true; 
      ConvertStrToValue<ParamType>(params.ParamValues[i], Value);
    }

    if (!bFound) {
      if (bRequired) {
        // Here the program terminates
        LOG(FATAL) << "Mandatory parameter: " << Name << " is missing!";
      }
    }
    LOG(INFO) << "@@@ Parameter: " << Name << "=" << Value << " @@@";
    seen.insert(Name);
  }

  template <typename ParamType>
  void ConvertStrToValue(const string& s, ParamType& Value);
};


template <typename ParamType>
inline void AnyParamManager::ConvertStrToValue(const string& s, ParamType& Value) {
  stringstream str(s);

  if (!(str>>Value) || !str.eof()) {
    LOG(FATAL) << "Failed to convert value '" << s << "' from type: " << typeid(Value).name();
  }
}

template <>
inline void AnyParamManager::ConvertStrToValue<string>(const string& str, string& Value) {
  Value = str;
}

void ParseCommandLine(int argc, char*av[],
                      string&             DistType,
                      string&             SpaceType,
                      unsigned&           dimension,
                      unsigned&           ThreadTestQty,
                      bool&               DoAppend, 
                      string&             ResFilePrefix,
                      unsigned&           TestSetQty,
                      string&             DataFile,
                      string&             QueryFile,
                      unsigned&           MaxNumData,
                      unsigned&           MaxNumQuery,
                      vector<unsigned>&   knn,
                      float&              eps,
                      string&             RangeArg,
                      multimap<string, AnyParams*>& Methods);

};

#endif
