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
 * File:   TextureArray.h
 * Author: jhendl
 *
 * Created on May 2, 2021, 2:21 PM
 */

#pragma once
#include <NyxGPU/vkg/Vulkan.h>
#include <vector>
#include <string>
#include <unordered_map>

namespace mars
{ 
  template<typename Framework>
  class Texture ;
  
  /** Object for managing a GPU texture array.
   */
  template<typename Framework>
  class TextureArray
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
          virtual void callback() = 0 ;
      };
      
      template<typename Object>
      class MethodCallback : public Callback
      {
        public:
          using MCallback = void(Object::*)() ;
          
          MethodCallback( Object* obj, MCallback cb ) ;
          
          virtual void callback() ;
        private:
          Object*   object     ;
          MCallback m_callback ;
      };

      
      class FunctionCallback : public Callback
      {
        public:
          using FCallback = void(*)() ;
          
          FunctionCallback( FCallback cb ) ;
          
          virtual void callback() ;
        private:
          FCallback m_callback ;
      };

      /** Method to initialize this object.
       */
      inline static void initialize( unsigned size ) ;
      
      /** Method to set a specific texture in this object's slots.
       * @param slot The slot to set
       * @param texture The texture to assign to that specific slot.
       */
      inline static void set( unsigned slot, mars::Texture<Framework>& texture ) ;
      
      /** Static method to add a callback to use to signal when this object gets updated.
       * @param object The object the callback belongs to.
       * @param callback The callback to call when a request is made.
       * @param key The key to associate with this callback.
       */
      template<typename Object>
      static void addCallback( Object* object, void (Object::*callback)(), const char* key ) ;
      
      /** Static method to add a callback to use to signal when this object gets updated.
       * @param callback The callback to call when a request is made.
       * @param key The key to associate with this callback.
       */
      static void addCallback( void (*callback)(), const char* key ) ;
      
      /** Static method to signal that this object has changed.
       */
      static void signal() ;
      
      /** Static method to remove a callback from this object.
       * @param key The key representing the callback.
       */
      static void removeCallback( const char* key ) ;

      /** Method to retrieve the size of this texture array.
       * @return The size of this object.
       */
      inline static unsigned count() ;
      
      /** Method to retrieve an array of image references to this object's data.
       * @return The array of image references to this object's data.
       */
      inline static const nyx::Image<Framework>* const* images() ;
      
    private:
      
      
      /** Constructor
       */
      TextureArray() ;
      
      /** Deconstructor
       */
      ~TextureArray() ;
      
      static std::vector<const nyx::Image<Framework>*> d_images ;
      
      /** Static member to contain this object's data.
       */
      static std::unordered_map<std::string, Callback*> map ;
  };
  template<typename Framework>
  using Callback = typename TextureArray<Framework>::Callback ;
  
  template<typename Framework>
  std::vector<const nyx::Image<Framework>*> TextureArray<Framework>::d_images ;
  
  template<typename Framework>
  std::unordered_map<std::string, Callback<Framework>*> TextureArray<Framework>::map ;
  
  template<typename Framework>
  void TextureArray<Framework>::initialize( unsigned size )
  {
    TextureArray<Framework>::d_images.resize( size ) ;
  }

  template<typename Framework>
  void TextureArray<Framework>::set( unsigned slot, mars::Texture<Framework>& texture )
  {
    if( slot < TextureArray<Framework>::d_images.size() ) TextureArray<Framework>::d_images[ slot ] = texture.pointer() ;
  }
  
  template<typename Framework>
  template<typename Object>
  TextureArray<Framework>::MethodCallback<Object>::MethodCallback( Object* obj, MCallback cb )
  {
    this->object     = obj ;
    this->m_callback = cb  ;
  }
      
  template<typename Framework>
  template<typename Object>
  void TextureArray<Framework>::MethodCallback<Object>::callback()
  {
    ( ( this->object )->*( this->m_callback ) )() ;
  }
  

  template<typename Framework>
  TextureArray<Framework>::FunctionCallback::FunctionCallback( FCallback cb )
  {
    this->m_callback = cb ;
  }
      
  template<typename Framework>
  void TextureArray<Framework>::FunctionCallback::callback()
  {
    ( ( this->m_callback ) )() ;
  }

  template<typename Framework>
  template<typename Object>
  void TextureArray<Framework>::addCallback( Object* object, void (Object::*callback)(), const char* key )
  {
    using Callback = TextureArray<Framework>::MethodCallback<Object> ;
    using Parent   = TextureArray<Framework> ;
    
    Callback* cb = new Callback( object, callback ) ;
    
    auto iter = Parent::map.find( key ) ;
    if( iter == Parent::map.end() ) 
    {
      Parent::map.insert( { std::string( key ), cb } ) ;
    }
    else
    {
      delete cb ;
    }
  }
  
  template<typename Framework>
  void TextureArray<Framework>::addCallback( void (*callback)(), const char* key )
  {
    using Callback = TextureArray<Framework>::FunctionCallback ;
    using Parent   = TextureArray<Framework>                   ;
    
    Callback* cb = new Callback( callback ) ;
    
    auto iter = Parent::map.find( key ) ;
    if( iter == Parent::map.end() ) 
    {
      Parent::map.insert( { std::string( key ), cb } ) ;
    }
    else
    {
      delete cb ;
    }
  }
  
  template<typename Framework>
  void TextureArray<Framework>::signal()
  {
    using Parent = TextureArray<Framework> ;
    
    for( auto& cb : Parent::map )
    {
      cb.second->callback() ;
    }
  }

  
  template<typename Framework>
  void TextureArray<Framework>::removeCallback( const char* key )
  {
    using Parent = TextureArray<Framework> ;
    auto iter = Parent::map.find( key ) ;
    if( iter != Parent::map.end() )
    {
      delete iter->second ;
      Parent::map.erase( iter ) ;
    }
  }
  
  template<typename Framework>
  unsigned TextureArray<Framework>::count()
  {
    return TextureArray<Framework>::d_images.size() ;
  }

  template<typename Framework>
  const nyx::Image<Framework>* const* TextureArray<Framework>::images()
  {
    return TextureArray<Framework>::d_images.data() ;
  }
}

