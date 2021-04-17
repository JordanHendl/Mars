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
 * Created on March 7, 2021, 5:11 PM
 */

#pragma once

/** Shouldn't have to worry about ABI because this is header only... I think.
 */
#include <stack>
#include <memory>
#include <mutex>
#include "Mars.h"

namespace mars
{
  /** Forward declare for factory friendship.
   */
  template<typename Type>
  class Factory ;
  
  /** Forward declare for manager friendship.
   */
  template<typename Key, typename Type>
  class Manager ;
  
  /** Wrapper object for a object retrieved from the factory.
   * @note This is used over just a shared pointer in case more library meta-data is needed.
   */
  template<typename Type>
  class Data
  {
    public:
      
      /** The amount of references of this data.
       * @return The amount of references for this data.
       */
      unsigned count() const ;

      /** Conversion operator to boolean to check for validation.
       * @return True if this object holds a valid reference. ( initialized & not de-referenced. )
       */
      operator bool() const ;
      
      /** Arrow overload to access underlying reference functions.
       * @return Pointer to this object's underlying reference.
       * @note Forwards a Mars library error on invalid access.
       */
      Type* operator->() ;
      
      /** Arrow overload to access underlying reference functions.
       * @return Const Pointer to this object's underlying reference.
       * @note Forwards a Mars library error on invalid access.
       */
      const Type* operator->() const ;
      
      /** Star overload to access a reference to this object's underlying data.
       * @return Reference to this object's underlying data.
       * @note Forwards a Mars library error on invalid access.
       */
      Type& operator*() ;
      
      /** Star overload to access a const reference to this object's underlying data.
       * @return Const reference to this object's underlying data.
       * @note Forwards a Mars library error on invalid access.
       */
      const Type& operator*() const ;
      
      /** Deconstructor to allow saving off these objects and letting them die. Does not re-add this data back into the factory.
       */
      ~Data() ;

    private:
      
      /** Static dummy object for invalid access returns.
       */
      static Type dummy ;
      
      /** Friend decleration so the factory can access this object.
       */
      template<typename Type2>
      friend class Factory ;
      
      template<typename Key, typename Type2>
      friend class Manager ;
      
      /** Privated constructor so only the factory can create copies of this object.
       */
      Data() ;
      
      /** Underlying shared pointer holding this object's data.
       */
      std::shared_ptr<Type> m_ptr ;
  };
  
  /** Static template object for cacheing objects in RAM.
   * @tparam Type The type of factory to access.
   */
  template<typename Type>
  class Factory
  {
    public:
      
      /** Static method for retrieving an object from the factory.
       * @param params The parameters to use for initializing the retrieved object.
       * @return A Wrapped up reference to the created object.
       */
      template<typename ... Parameters>
      static Data<Type> create( Parameters... params ) ;
      
      /** Static method for destroying/reusing an object from the factory.
       * @param data The data reference to put back into the factory.
       */
      static void destroy( Data<Type>& data ) ;
      
      /** Static method to cleanup the factory.
       * @Note Usage of this should be use sparingly, as it frees all allocated data of the factory and returns it to minimum settings.
       *       For example, if you use a LOT of factory objects in a scene, but use next to none in the next scene, the previous cache will exist if cleanup is not called.
       */
      static void cleanup() ;
    private :

      /** The base, minimum size of the underlying stack.
       */
      static constexpr unsigned MIN_SIZE = 10 ;
      
      /** Alias for a stack.
       */
      using DataStack  = std::stack<Data<Type>> ;
      
      /** The stack to use for containing the cache.
       */
      static DataStack  stack ;
      
      /** The mutex to ensure thread safety.
       */
      static std::mutex stack_lock ;
      
      /** Constructing is disallowed.
       */
      Factory()  = delete ;
      
      /** Deconstructing is disallowed.
       */
      ~Factory() = delete ;
  };
  
  template<typename Type>
  std::stack<Data<Type>> Factory<Type>::stack ;
  
  template<typename Type>
  std::mutex Factory<Type>::stack_lock ;
  
  template<typename Type>
  Type Data<Type>::dummy ;

  template<typename Type>
  template<typename ... Parameters>
  Data<Type> Factory<Type>::create( Parameters... params )
  {
    Data<Type> data ;
    
    if( Factory<Type>::stack.empty() )
    {
      for( unsigned index = 0; index < Factory<Type>::MIN_SIZE; index++ )
      {
        data = Data<Type>() ;
        data.m_ptr = std::make_shared<Type>() ;
        Factory<Type>::stack.push( data ) ;
      }
    }
    
    Factory<Type>::stack_lock.lock() ;
    
    data = Factory<Type>::stack.top() ;
    Factory<Type>::stack.pop() ;
    
    Factory<Type>::stack_lock.unlock() ;
    
    data->initialize( params... ) ;
    return data ;
  }
  
  template<typename Type>
  void Factory<Type>::destroy( mars::Data<Type>& data )
  {
    data->reset() ;
    
    Factory<Type>::stack_lock.lock() ;
    Factory<Type>::stack.push( data ) ;
    Factory<Type>::stack_lock.unlock() ;
    
    data.m_ptr = nullptr ;
  }
  
  template<typename Type>
  void Factory<Type>::cleanup()
  {
    Factory<Type>::stack_lock.lock() ;
    while( Factory<Type>::stack.size() > Factory<Type>::MIN_SIZE )
    {
      Factory::stack.pop() ; // Shared pointer should deallocate upon reference loss.
    }
    Factory<Type>::stack_lock.unlock() ;
  }

  template<typename Type>
  Data<Type>::Data()
  {
    this->m_ptr = nullptr ;
  }

  template<typename Type>
  Data<Type>::~Data()
  {
    this->m_ptr = nullptr ;
  }
  
  template<typename Type>
  unsigned Data<Type>::count() const
  {
    return this->m_ptr.use_count() ;
  }

  template<typename Type>
  Data<Type>::operator bool() const
  {
    return this->m_ptr && this->m_ptr->initialized() ;
  }
  
  template<typename Type>
  Type* Data<Type>::operator->()
  {
    if( this->m_ptr ) return this->m_ptr.get() ;
    mars::handleError( __FILE__, __LINE__, mars::Error::InvalidAccess ) ;
    return &Data<Type>::dummy ;
  }

  template<typename Type>
  const Type* Data<Type>::operator->() const
  {
    if( this->m_ptr ) return this->m_ptr.get() ;
    mars::handleError( __FILE__, __LINE__, mars::Error::InvalidAccess ) ;
    return &Data<Type>::dummy ;
  }

  template<typename Type>
  Type& Data<Type>::operator*()
  {
    if( this->m_ptr ) return *this->m_ptr ;
    mars::handleError( __FILE__, __LINE__, mars::Error::InvalidAccess ) ;
    return Data<Type>::dummy ;
  }

  template<typename Type>
  const Type& Data<Type>::operator*() const
  {
    
    if( this->m_ptr ) return *this->m_ptr ;
    mars::handleError( __FILE__, __LINE__, mars::Error::InvalidAccess ) ;
    return Data<Type>::dummy ;
  }
}
