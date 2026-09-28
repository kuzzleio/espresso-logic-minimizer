#include <napi.h>
#include <stdlib.h>
#include <string.h>
#include <string>

extern "C" char ** run_espresso_from_data(char ** data, unsigned int length);
extern "C" char ** run_espresso_from_path(char * path);

// Moves a NULL-terminated array of strings returned by espresso into a JS
// array. Since the result comes from C code, the memory was allocated using
// malloc and must be freed with free.
static Napi::Array toArray(Napi::Env env, char ** result) {
  Napi::Array returnValue = Napi::Array::New(env);

  if (result != NULL) {
    for(uint32_t i = 0; result[i] != NULL; ++i) {
      returnValue.Set(i, Napi::String::New(env, result[i]));
      free(result[i]);
    }

    free(result);
  }

  return returnValue;
}

Napi::Value minimize_from_data(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  Napi::Array array = info[0].As<Napi::Array>();
  unsigned int length = array.Length();
  char **truthTable;

  // returns an empty array if no input is provided
  if (length == 0) {
    return Napi::Array::New(env);
  }

  truthTable = new char*[length];

  for(unsigned int i = 0; i < length; ++i) {
    std::string val = array.Get(i).As<Napi::String>().Utf8Value();
    truthTable[i] = new char[val.size() + 1];
    strcpy(truthTable[i], val.c_str());
  }

  Napi::Array returnValue = toArray(env, run_espresso_from_data(truthTable, length));

  // memory clean up
  for(unsigned int i = 0; i < length; delete[] truthTable[i++]);
  delete[] truthTable;

  return returnValue;
}

Napi::Value minimize_from_path(const Napi::CallbackInfo& info) {
  Napi::Env env = info.Env();
  std::string path = info[0].As<Napi::String>().Utf8Value();

  // returns an empty array if no path
  if (path.empty()) {
    return Napi::Array::New(env);
  }

  return toArray(env, run_espresso_from_path(&path[0]));
}

Napi::Object init(Napi::Env env, Napi::Object exports) {
  exports.Set("minimize_from_data", Napi::Function::New(env, minimize_from_data));
  exports.Set("minimize_from_path", Napi::Function::New(env, minimize_from_path));
  return exports;
}

NODE_API_MODULE(EspressoLogicMinimizer, init)
