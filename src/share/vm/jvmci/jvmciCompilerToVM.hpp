/*
 * Copyright (c) 2011, 2013, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 */

#ifndef SHARE_VM_JVMCI_JVMCI_COMPILER_TO_VM_HPP
#define SHARE_VM_JVMCI_JVMCI_COMPILER_TO_VM_HPP

#include "prims/jni.h"
#include "runtime/javaCalls.hpp"
#include "jvmci/jvmciJavaAccess.hpp"

class CompilerToVM {
public:
  /**
   * Tag bits used by lookupKlassInPool to distinguish the types in Java.
   */
  enum Tags {
    KLASS_TAG = 0x0,
    SYMBOL_TAG = 0x1
  };

  static intptr_t tag_pointer(Klass* klass) {
    return ((intptr_t) klass) | KLASS_TAG;
  }

  static intptr_t tag_pointer(Symbol* symbol) {
    return ((intptr_t) symbol) | SYMBOL_TAG;
  }

  static JNINativeMethod methods[];
  static int methods_count();
  
  static inline Method* asMethod(jobject jvmci_method) {
    return (Method*) (address) HotSpotResolvedJavaMethodImpl::metaspaceMethod(jvmci_method);
  }
  
  static inline Method* asMethod(Handle jvmci_method) {
    return (Method*) (address) HotSpotResolvedJavaMethodImpl::metaspaceMethod(jvmci_method);
  }
  
  static inline Method* asMethod(oop jvmci_method) {
    return (Method*) (address) HotSpotResolvedJavaMethodImpl::metaspaceMethod(jvmci_method);
  }
  
  static inline ConstantPool* asConstantPool(jobject jvmci_constant_pool) {
    return (ConstantPool*) (address) HotSpotConstantPool::metaspaceConstantPool(jvmci_constant_pool);
  }
  
  static inline ConstantPool* asConstantPool(Handle jvmci_constant_pool) {
    return (ConstantPool*) (address) HotSpotConstantPool::metaspaceConstantPool(jvmci_constant_pool);
  }
  
  static inline ConstantPool* asConstantPool(oop jvmci_constant_pool) {
    return (ConstantPool*) (address) HotSpotConstantPool::metaspaceConstantPool(jvmci_constant_pool);
  }
  
  static inline Klass* asKlass(jobject jvmci_type) {
    return java_lang_Class::as_Klass(HotSpotResolvedObjectTypeImpl::javaClass(jvmci_type));
  }

  static inline Klass* asKlass(Handle jvmci_type) {
    return java_lang_Class::as_Klass(HotSpotResolvedObjectTypeImpl::javaClass(jvmci_type));
  }
  
  static inline Klass* asKlass(oop jvmci_type) {
    return java_lang_Class::as_Klass(HotSpotResolvedObjectTypeImpl::javaClass(jvmci_type));
  }
  
  static inline MethodData* asMethodData(jlong metaspaceMethodData) {
    return (MethodData*) (address) metaspaceMethodData;
  }
  
  static oop get_jvmci_method(methodHandle method, TRAPS);

  static oop get_jvmci_type(KlassHandle klass, TRAPS);
};

class JavaArgumentUnboxer : public SignatureIterator {
 protected:
  JavaCallArguments*  _jca;
  arrayOop _args;
  int _index;

  oop next_arg(BasicType expectedType) {
    assert(_index < _args->length(), "out of bounds");
    oop arg=((objArrayOop) (_args))->obj_at(_index++);
    assert(expectedType == T_OBJECT || java_lang_boxing_object::is_instance(arg, expectedType), "arg type mismatch");
    return arg;
  }

 public:
  JavaArgumentUnboxer(Symbol* signature, JavaCallArguments*  jca, arrayOop args, bool is_static) : SignatureIterator(signature) {
    this->_return_type = T_ILLEGAL;
    _jca = jca;
    _index = 0;
    _args = args;
    if (!is_static) {
      _jca->push_oop(next_arg(T_OBJECT));
    }
    iterate();
    assert(_index == args->length(), "arg count mismatch with signature");
  }

  inline void do_bool()   { if (!is_return_type()) _jca->push_int(next_arg(T_BOOLEAN)->bool_field(java_lang_boxing_object::value_offset_in_bytes(T_BOOLEAN))); }
  inline void do_char()   { if (!is_return_type()) _jca->push_int(next_arg(T_CHAR)->char_field(java_lang_boxing_object::value_offset_in_bytes(T_CHAR))); }
  inline void do_short()  { if (!is_return_type()) _jca->push_int(next_arg(T_SHORT)->short_field(java_lang_boxing_object::value_offset_in_bytes(T_SHORT))); }
  inline void do_byte()   { if (!is_return_type()) _jca->push_int(next_arg(T_BYTE)->byte_field(java_lang_boxing_object::value_offset_in_bytes(T_BYTE))); }
  inline void do_int()    { if (!is_return_type()) _jca->push_int(next_arg(T_INT)->int_field(java_lang_boxing_object::value_offset_in_bytes(T_INT))); }

  inline void do_long()   { if (!is_return_type()) _jca->push_long(next_arg(T_LONG)->long_field(java_lang_boxing_object::value_offset_in_bytes(T_LONG))); }
  inline void do_float()  { if (!is_return_type()) _jca->push_float(next_arg(T_FLOAT)->float_field(java_lang_boxing_object::value_offset_in_bytes(T_FLOAT))); }
  inline void do_double() { if (!is_return_type()) _jca->push_double(next_arg(T_DOUBLE)->double_field(java_lang_boxing_object::value_offset_in_bytes(T_DOUBLE))); }

  inline void do_object() { _jca->push_oop(next_arg(T_OBJECT)); }
  inline void do_object(int begin, int end) { if (!is_return_type()) _jca->push_oop(next_arg(T_OBJECT)); }
  inline void do_array(int begin, int end)  { if (!is_return_type()) _jca->push_oop(next_arg(T_OBJECT)); }
  inline void do_void()                     { }
};

#endif // SHARE_VM_JVMCI_JVMCI_COMPILER_TO_VM_HPP