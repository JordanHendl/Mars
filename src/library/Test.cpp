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
 * File:   Test.cpp
 * Author: jhendl
 *
 * Created on March 9, 2021, 3:51 PM
 */

#include <athena/Manager.h>
#include "Factory.h"
#include "Manager.h"
#include <string>
#include <iostream>

namespace mars
{
  template<typename Framework>
  class Model
  {
    public: 
      Model() = default ;
      
      /** These 3 functions are all required for factory usage.
       */
      void initialize() {this->initted = true ; } ;
      bool initialized() const { return this->initted ; } ;
      void reset() { this->initted = false ; } ; 
      

    private:
      bool initted = false ;
  };
  
  class Impl
  {
    public: Impl() = default ;
  };
  
  athena::Result test_manager()
  {
    using Model   = mars::Model  <Impl           > ;
    using Manager = mars::Manager<unsigned, Model> ;
    
    mars::Reference<Model> ref  = Manager::create   ( 0 ) ;
    mars::Reference<Model> ref2 = Manager::reference( 0 ) ;
    mars::Reference<Model> ref3 = Manager::reference( 0 ) ;
    
    if( ref.count() != 4 ) return false ;
    
    ref2.~Data() ;
    
    if( ref.count() != 3 ) return false ;
    
    ref3.~Data() ;
    
    if( ref.count() != 2 ) return false ;
    
    ref.~Data() ;
    
    Manager::cleanup() ;
    
    return true ;
  }

  athena::Result test_factory()
  {
    using Model   = mars::Model  <Impl > ;
    using Factory = mars::Factory<Model> ;
    
    auto model = Factory::create() ;
    
    if( !model || model.count() != 1 ) return false ;
    
    Factory::destroy( model ) ;
    
    if( model || model.count() != 0 ) return false ;
    
    Factory::cleanup() ;
    
    return true ;
  }
}

int main()
{
  athena::Manager manager ;
  manager.initialize( "Mars Library Test" ) ;
  
  manager.add( "Factory Test", &mars::test_factory ) ;
  manager.add( "Manager Test", &mars::test_manager ) ;
  return manager.test( athena::Output::Verbose ) ;
}