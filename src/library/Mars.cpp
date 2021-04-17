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
 * File:   Mars.cpp
 * Author: jhendl
 * 
 * Created on March 9, 2021, 3:49 PM
 */

#include "Mars.h"
#include "Manager.h"
#include <iostream>

namespace mars
{
  #if defined ( __unix__ ) || defined( _WIN32 )
    constexpr const char* END_COLOR    = "\x1B[m"     ;
    constexpr const char* COLOR_RED    = "\u001b[31m" ;
    constexpr const char* COLOR_GREEN  = "\u001b[32m" ;
    constexpr const char* COLOR_YELLOW = "\u001b[33m" ;
    constexpr const char* COLOR_GREY   = "\x1B[1;30m" ;
    constexpr const char* UNDERLINE    = "\u001b[4m"  ;
  #else
    constexpr const char* END_COLOR    = "" ;
    constexpr const char* COLOR_GREEN  = "" ;
    constexpr const char* COLOR_YELLOW = "" ;
    constexpr const char* COLOR_GREY   = "" ;
    constexpr const char* COLOR_RED    = "" ;
    constexpr const char* COLOR_WHITE  = "" ;
  #endif

  /** The structure to contain all of the global nyx library data.
   */
  struct NyxData
  {
    typedef void ( *Callback )( const char*, unsigned, mars::Error ) ;

    Callback           error_cb ;
    mars::ErrorHandler* handler  ;

    /** Default constructor.
     */
    NyxData() ;
  };

  /** Static container for all of the nyx library's global data.
   */
  static NyxData data ;

  const char* colorFromSeverity( mars::Severity severity )
  {
    switch ( severity )
    {
      case mars::Severity::Info    : return mars::COLOR_GREY   ;
      case mars::Severity::Warning : return mars::COLOR_YELLOW ;
      case mars::Severity::Fatal   : return mars::COLOR_RED    ;
      default : return mars::COLOR_RED ;
    }
  }

  void defaultHandler( const char* file, unsigned line, mars::Error error )
  {
    auto severity = error.severity() ;
    std::cout << colorFromSeverity( severity ) << "--" << severity.toString() << " in file" << file << " : " << line << "  " << error.toString() << mars::END_COLOR << std::endl ;
    if( severity == mars::Severity::Fatal ) exit( -1 ) ;
  }

  NyxData::NyxData()
  {
    this->error_cb = &mars::defaultHandler ;
    this->handler  = nullptr ;
  }

  mars::Severity::Severity()
  {
    this->sev = Severity::None ;
  }

  mars::Severity::Severity( const mars::Severity& severity )
  {
    this->sev = severity.sev ;
  }

  mars::Severity::Severity( unsigned error )
  {
    this->sev = error ;
  }

  unsigned mars::Severity::severity() const
  {
    return *this ;
  }

  mars::Severity::operator unsigned() const
  {
    return this->sev ;
  }

  const char* mars::Severity::toString() const
  {
    switch( this->sev )
    {
      case Severity::Warning : return "Warning" ;
      case Severity::Fatal   : return "Fatal"   ;
      case Severity::None    : return "None"    ;
      case Severity::Info    : return "Info"    ;
      default : return "Unknown Severity" ;
    }
  }

  mars::Error::Error()
  {
    this->err = Error::None ;
  }

  mars::Error::Error( const mars::Error& error )
  {
    this->err = error.err ;
  }

  mars::Error::Error( unsigned error )
  {
    this->err = error ;
  }

  unsigned mars::Error::error() const
  {
    return *this ;
  }

  mars::Error::operator unsigned() const
  {
    return this->err ;
  }

  const char* mars::Error::toString() const
  {
    switch( this->err )
    {
      case mars::Error::InvalidReference : return "An invalid reference was requested."                   ;
      case mars::Error::DoubleReference  : return "An reference was requested to be created twice."       ;
      case mars::Error::InvalidAccess    : return "An invalid access of a reference/data object occured." ;
      default : return "Unknown Error" ;
    }
  }

  mars::Severity mars::Error::severity() const
  {
    switch( this->err )
    {
      case mars::Error::DoubleReference  : return mars::Severity::Warning ;
      case mars::Error::InvalidReference : return mars::Severity::Fatal   ;
      case mars::Error::InvalidAccess    : return mars::Severity::Fatal   ;
      default : return mars::Severity::Fatal ;
    }
  }

  void handleError(  const char* file, unsigned line, mars::Error error )
  {
    if( error != mars::Error::Success )
    {
      if( data.error_cb != nullptr )
      {
        ( data.error_cb )( file, line, error ) ;
      }

      if( data.handler != nullptr )
      {
        data.handler->handleError( file, line, error ) ;
      }
    }
  }
  void setErrorHandler( void ( *error_handler )( const char*, unsigned, mars::Error ) )
  {
    data.error_cb = error_handler ;
  }
  
  void setErrorHandler( mars::ErrorHandler* handler )
  {
    data.handler = handler ;
  }
}
