#include <napi.h>
#include <cstdlib>
#include <cstring>

extern "C" char ** run_espresso_from_data(char ** data, unsigned int length);
extern "C" char ** run_espresso_from_path(char * path);

Napi::Array minimize_from_data(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Array array = info[0].As<Napi::Array>();
  Napi::Array returnValue = Napi::Array::New(env);
  unsigned int length = array.Length();
  char
    **truthTable,
    **result;

  // returns an empty array if no input is provided
  if (length == 0) {
    return returnValue;
  }

  truthTable = new char*[length];

  for(unsigned int i = 0; i < length; ++i) {
    std::string val = array.Get(i).As<Napi::String>().Utf8Value();
    truthTable[i] = new char[val.size() + 1];
    strcpy(truthTable[i], val.c_str());
  }

  result = run_espresso_from_data(truthTable, length);

  if (result != NULL) {
    for(unsigned int i = 0; result[i] != NULL; ++i) {
      returnValue.Set(i, Napi::String::New(env, result[i]));

      // since the result comes from C code, the memory was
      // allocated using malloc and must be freed with free
      free(result[i]);
    }

    free(result);
  }

  // memory clean up
  for(unsigned int i = 0; i < length; ++i) {
    delete[] truthTable[i];
  }
  delete[] truthTable;

  return returnValue;
}

Napi::Array minimize_from_path(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::string path = info[0].As<Napi::String>().Utf8Value();
  Napi::Array returnValue = Napi::Array::New(env);
  char **result;

  // returns an empty array if no path
  if (path.empty()) {
    return returnValue;
  }

  result = run_espresso_from_path(const_cast<char*>(path.c_str()));

  if (result != NULL) {
    for(unsigned int i = 0; result[i] != NULL; ++i) {
      returnValue.Set(i, Napi::String::New(env, result[i]));

      // since the result comes from C code, the memory was
      // allocated using malloc and must be freed with free
      free(result[i]);
    }

    free(result);
  }

  return returnValue;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set("minimize_from_data", Napi::Function::New(env, minimize_from_data));
  exports.Set("minimize_from_path", Napi::Function::New(env, minimize_from_path));
  return exports;
}

NODE_API_MODULE(NODE_GYP_MODULE_NAME, Init)
