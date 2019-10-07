
#ifndef _SERIALRPC_H_
#define _SERIALRPC_H_

#include <Arduino.h>

enum struct ParameterDirection {
  INPUT_PARAM = 'i',
  OUTPUT_PARAM = 'o',
  EXCHANGE_PARAM = 'x'
};

class Parameter {
public:
  ParameterDirection direction;
  boolean ref;
  boolean arr;
  boolean str;
  unsigned int size;
  unsigned int data;

  Parameter(boolean _ref, char _direction, unsigned int _data, unsigned int _size) {
    direction = (_direction == 's') ? ParameterDirection::INPUT_PARAM : (ParameterDirection) tolower(_direction);
    ref = _ref;
    str = (_direction == 's');
    arr = (_direction < 'a');
    data = _data;
    size = _size;
  };

  void print() {
    Serial.print((char) direction);
    Serial.print(" ");
    if (str) {
      Serial.print((const char*) data);
    } else if (ref) {
      Serial.print("->");
    } else if (arr) {
      Serial.print("[]");
    } else {
      Serial.print(data);
    }
    Serial.print(" ");
    Serial.println(size);
  };

};

class Parameters {
public:
  const char* directions;
  Parameter *params[10];
  int l = 0;

  Parameters(const char* _directions) {
    directions = _directions;
  }

  ~Parameters() {
    for (int i = 0; i < l; i++) {
      delete params[i];
    }
  }

  template<typename T>
  inline void addParam(T param) {
    params[l] = new Parameter(false, directions[l], (unsigned int) param, sizeof(T));
    l++;
  }

  template<typename T>
  inline void addParam(T* param) {
    params[l] = new Parameter(true, directions[l], (unsigned int) param, sizeof(T));
    l++;
  }

  template<typename T, typename... Args>
  inline void addParam(T param,  Args... args) {
    params[l] = new Parameter(false, directions[l], (unsigned int) param, sizeof(T));
    l++;
    addParam(args...);
  }

  template<typename T, typename... Args>
  inline void addParam(T* param,  Args... args) {
    params[l] = new Parameter(true, directions[l], (unsigned int) param, sizeof(T));
    l++;
    addParam(args...);
  }

  void print() {
    Serial.println(l);
    for (int i = 0; i < l; i++) {
      if (params[i]->arr) {
        params[i]->size = params[i]->size * params[i+1]->data;
      } else if (params[i]->str) {
        params[i]->size = strlen((const char*) params[i]->data);
      }
      params[i]->print();
    }
  }
};

template<typename... Args>
void call(byte functionIndex, const char* directions, Args... args) {
  Parameters params(directions);
  params.addParam(args...);
  Serial.println(functionIndex);
  params.print();
}

#endif
