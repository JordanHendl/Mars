/*
 * Copyright (C) 2020 Jordan Hendl
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * File:   Manager.h
 * Author: jhendl
 *
 * Created on March 9, 2021, 4:41 PM
 */

#pragma once

/** Shouldn't have to worry about ABI because this is header only... I think.
 */
#include "Factory.h"
#include "Mars.h"
#include <unordered_map>

namespace mars
{
  /** Alias a Data object from Factory.h.
   */
  template<typename Type>
  using Reference = Data<Type> ;
  
  
  /** Static template object for containing and referencing data.
   * @tparam Type The type of data to manage.
   */
  template<typename Key, typename Type>
  class Manager
  {
    public:
      /** Class for publishing data through function pointers AKA 'getters'.
       */
      class Callback
      {
        public:
          /** Virtual deconstructor.
           */
          virtual ~Callback() {} ;
          
          /** Method to 
           * @param index The index to use for publishing.
           * @return The internal function's published data.
           */
          virtual void callback( Key key, mars::Reference<Type> reference ) = 0 ;
      };
      
      template<typename Object>
      class MethodCallback : public Callback
      {
        public:
          using MCallback = void(Object::*)( Key, mars::Reference<Type> ) ;
          
          MethodCallback( Object* obj, MCallback cb ) ;
          
          virtual void callback( Key key, mars::Reference<Type> reference ) ;
        private:
          Object*   object     ;
          MCallback m_callback ;
      };

      
      class FunctionCallback : public Callback
      {
        public:
          using FCallback = void(*)( Key, mars::Reference<Type> ) ;
          
          FunctionCallback( FCallback cb ) ;
          
          virtual void callback( Key key, mars::Reference<Type> reference ) ;
        private:
          FCallback m_callback ;
      };
      
      /** Static method to retrieve a reference of the value of this object at the specified key.
       * @note Forwards a library warning on invalid access.
       * @param key The key to retrieve the reference of.
       * @return The reference of the key if it exists; an empty reference otherwise.
       */
      static Reference<Type> reference( const Key& key ) ;
      
      /** Static method to check and see if a type is in the manager.
       * @param key The key to look for.
       * @return Whether or not there is a value at the key.
       */
      static bool has( const Key& key ) ;
      
      /** Static method to request data to be loaded to the manager.
       * @param object The object the callback belongs to.
       * @param callback The callback to call when the request has been fulfilled.
       * @param key The key to load.
       */
      template<typename Object>
      static void request( Object* object, void (Object::*callback)( Key, mars::Reference<Type> ), Key key ) ;
      
      /** Static method to request data to be loaded to the manager.
       * @param callback The callback to call when the request has been fulfilled.
       * @param key The key to load.
       */
      static void request( void (*callback)( Key, mars::Reference<Type> ), Key key ) ;
      
      /** Static method to add a callback to use to fullfill requests to the manager.
       * @param object The object the callback belongs to.
       * @param callback The callback to call when a request is made.
       * @param key The key to associate with this fulfiller.
       */
      template<typename Object>
      static void addFulfiller( Object* object, void (Object::*callback)( Key, Callback* ), Key key ) ;
      
      /** Static method to add a callback to use to fullfill requests to the manager.
       * @param callback The callback to call when a request is made.
       * @param key The key to associate with this fulfiller.
       */
      static void addFulfiller( void (*callback)( Key, Callback* ), Key key ) ;
      
      /** Static method to remove a fulfiller.
       * @param key The key representing the fulfiller to remove.
       */
      static void removeFulfiller( Key key ) ;
      
      /** Static method to create an object and insert it into this object.
       * @param key The key to insert the object into, if possible.
       * @param params The parameters to use for initializing the object.
       * @return A reference to the created object.
       */
      template<typename ... Parameters>
      static Reference<Type> create( const Key& key, Parameters... params ) ;
      
      /** Static method to cleanup this object's leftover data.
       * Any data with no references will be reset and released.
       */
      static void cleanup() ;

      /** Class for publishing data through function pointers AKA 'getters'.
       */
      class Fulfiller
      {
        public:
          /** Virtual deconstructor.
           */
          virtual ~Fulfiller() {} ;
          
          /** Method to 
           * @param index The index to use for publishing.
           * @return The internal function's published data.
           */
          virtual void fulfill( Key key, Callback* callback ) = 0 ;
      };
    private:
      
      template<typename Object>
      class MethodFulfiller : public Fulfiller
      {
        public:
          using FCallback = void(Object::*)( Key, Callback* ) ;
          
          MethodFulfiller( Object* obj, FCallback cb ) ;
          
          virtual void fulfill( Key key, Callback* callback ) ;
        private:
          Object*   object   ;
          FCallback callback ;
      };

      
      class FunctionFulfiller : public Fulfiller
      {
        public:
          using FCallback = void(*)( Key, mars::Reference<Type> ) ;
          
          FunctionFulfiller( FCallback cb ) ;
          
          virtual void fulfill( Key key, Callback* callback ) ;
        private:
          FCallback callback ;
      };
      
      /** Static member to contain this object's data.
       */
      static std::unordered_map<Key, Reference<Type>> map ;
      
      /** Static member to contain fulfillers to fulfill requests.
       */
      static std::unordered_map<Key, Manager<Key, Type>::Fulfiller*> fullfillers ;
      
      /** Static member to ensure thread safety.
       */
      static std::mutex map_lock ;
      
      /** Creation is disallowed.
       */
      Manager() ;
      
      /** Destruction is disallowed.
       */
      ~Manager() ;
  };
  
  template<typename Key, typename Type>
  using Fullfiller = typename Manager<Key, Type>::Fulfiller ;
  
  template<typename Key, typename Type>
  std::unordered_map<Key, Reference<Type>> Manager<Key, Type>::map ;
  
  template<typename Key, typename Type>
  std::unordered_map<Key, Fullfiller<Key, Type>*> Manager<Key, Type>::fullfillers ;
  
  template<typename Key, typename Type>
  std::mutex Manager<Key, Type>::map_lock ;
  
  template<typename Key, typename Type>
  Reference<Type> Manager<Key, Type>::reference( const Key& key )
  {
    using Manager = Manager<Key, Type> ;
    Reference<Type> ref ;
    
    const auto iter = Manager::map.find( key ) ;
    
    if( iter != Manager::map.end() )
    {
      ref = iter->second ;
      
      if( !ref ) mars::handleError( __FILE__, __LINE__, mars::Error::InvalidReference ) ;
    }
    else
    {
      mars::handleError( __FILE__, __LINE__, mars::Error::InvalidReference ) ;
    }
    
    return ref ;
  }
  
  template<typename Key, typename Type>
  template<typename Object>
  Manager<Key, Type>::MethodCallback<Object>::MethodCallback( Object* obj, MCallback cb )
  {
    this->object     = obj ;
    this->m_callback = cb  ;
  }
      
  template<typename Key, typename Type>
  template<typename Object>
  void Manager<Key, Type>::MethodCallback<Object>::callback( Key key, mars::Reference<Type> reference )
  {
    ( ( this->object )->*( this->m_callback ) )( key, reference ) ;
  }
  

  template<typename Key, typename Type>
  Manager<Key, Type>::FunctionCallback::FunctionCallback( FCallback cb )
  {
    this->m_callback = cb ;
  }
      
  template<typename Key, typename Type>
  void Manager<Key, Type>::FunctionCallback::callback( Key key, mars::Reference<Type> reference )
  {
    ( ( this->m_callback ) )( key, reference ) ;
  }
  
  template<typename Key, typename Type>
  template<typename Object>
  Manager<Key, Type>::MethodFulfiller<Object>::MethodFulfiller( Object* obj, FCallback cb )
  {
    this->object   = obj ;
    this->callback = cb  ;
  }
      
  template<typename Key, typename Type>
  template<typename Object>
  void Manager<Key, Type>::MethodFulfiller<Object>::fulfill( Key key, Callback* callback )
  {
    ( ( this->object )->*( this->callback ) )( key, callback ) ;
  }

  
    template<typename Key, typename Type>
  Manager<Key, Type>::FunctionFulfiller::FunctionFulfiller( FCallback cb )
  {
    this->callback = cb  ;
  }
      
  template<typename Key, typename Type>
  void Manager<Key, Type>::FunctionFulfiller::fulfill( Key key, Callback* callback )
  {
    ( ( this->callback ) )( key, callback ) ;
  }

  template<typename Key, typename Type>
  template<typename Object>
  void Manager<Key, Type>::request( Object* object, void (Object::*callback)( Key, mars::Reference<Type> ), Key key )
  {
    using FCallback = Manager<Key, Type>::MethodCallback<Object> ;
    if( !Manager<Key, Type>::fullfillers.empty() )
    {
      FCallback* cb = new FCallback( object, callback ) ;
      
      Manager<Key, Type>::fullfillers.begin()->second->fulfill( key, cb ) ;
    }
  }
  
  template<typename Key, typename Type>
  void Manager<Key, Type>::request( void (*callback)( Key, mars::Reference<Type> ), Key key )
  {
    using FCallback = Manager<Key, Type>::FunctionCallback ;
    if( !Manager<Key, Type>::fullfillers.empty() )
    {
      FCallback* cb = new FCallback( callback ) ;
      
      Manager<Key, Type>::fullfillers.begin()->second->fulfill( key, cb ) ;
    }
  }
  
  template<typename Key, typename Type>
  template<typename Object>
  void Manager<Key, Type>::addFulfiller( Object* object, void (Object::*callback)( Key, Callback* ), Key key )
  {
    Manager<Key, Type>::Fulfiller* fullfiller = new Manager<Key, Type>::MethodFulfiller<Object>( object, callback ) ;
    auto iter = Manager<Key, Type>::fullfillers.find( key ) ;
    if( iter != Manager<Key, Type>::fullfillers.end() )
    {
      delete iter->second ;
    }
    
    Manager<Key, Type>::fullfillers[ key ] = fullfiller ;
  }
  
  template<typename Key, typename Type>
  void Manager<Key, Type>::addFulfiller( void (*callback)( Key, Callback* ), Key key )
  {
    Manager<Key, Type>::Fulfiller* fullfiller = new Manager<Key, Type>::FunctionFulfiller( callback ) ;
    auto iter = Manager<Key, Type>::fullfillers.find( key ) ;
    if( iter != Manager<Key, Type>::fullfillers.end() )
    {
      delete iter->second ;
    }
    
    Manager<Key, Type>::fullfillers[ key ] = fullfiller ;
  }
  
  template<typename Key, typename Type>
  void Manager<Key, Type>::removeFulfiller( Key key )
  {
    auto iter = Manager<Key, Type>::fullfillers.find( key ) ;
    if( iter != Manager<Key, Type>::fullfillers.end() )
    {
      delete iter->second ;
      Manager<Key, Type>::fullfillers.erase( iter ) ;
    }
  }
  
  template<typename Key, typename Type>
  bool Manager<Key, Type>::has( const Key& key )
  {
    using Manager = Manager<Key, Type> ;
    Reference<Type> ref ;
    
    const auto iter = Manager::map.find( key ) ;
    
    if( iter != Manager::map.end() )
    {
      return true ;
    }
    else
    {
      return false ;
    }
  }
  
  template<typename Key, typename Type>
  template<typename ... Parameters>
  Reference<Type> Manager<Key, Type>::create( const Key& key, Parameters ... params )
  {
    using Manager = Manager<Key, Type> ;
    const auto iter = Manager::map.find( key ) ;
    Reference<Type> ref ;
    
    if( iter != Manager::map.end() )
    {
      mars::handleError( __FILE__, __LINE__, mars::Error::DoubleReference ) ;
      ref = iter->second ;
    }
    else
    {
      ref.m_ptr = std::make_shared<Type>() ;
      ref.m_ptr->initialize( params... ) ;
      Manager::map.insert( iter, { key, ref } ) ;
    }

    return ref ;
  }
  
  template<typename Key, typename Type>
  void Manager<Key, Type>::cleanup()
  {
    using Manager = Manager<Key, Type> ;
    
    for( auto entry = Manager::map.begin(); entry != Manager::map.end(); ++entry )
    {
      if( entry->second.count() <= 1 )
      {
        entry->second->reset() ;
        Manager::map.erase( entry ) ;
      }
    }
  }
}
