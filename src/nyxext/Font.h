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
 * File:   Font.h
 * Author: jhendl
 *
 * Created on April 16, 2021, 11:53 PM
 */

#pragma once
#include "NyxGPU/NttFile.h"
#include "NyxGPU/library/Array.h"
#include "NyxGPU/library/Image.h"
#include "NyxGPU/library/Chain.h"
#include "NyxGPU/library/Pipeline.h"
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <map>

namespace mars
{
  /** Template font class. Acts as a 3D font, and provides functionality for drawing.
   */
  template<typename Framework>
  class Font
  {
    public:
      
      /** Constructor.
       */
      Font() ;
      
      /** Method to initialize this object.
       * @param font_path The path to the .ngg file on disk to load.
       * @param gpu the gpu to allocate the font on.
       */
      inline void initialize( const char* ntt_path, unsigned gpu ) ;
      
      /** Method to initialize this object.
       * @param bytes The bytes containing the .ngg file.
       * @param size  The size of the input bytes.
       * @param gpu the gpu to allocate the font on.
       */
      inline void initialize( unsigned char* bytes, unsigned size, unsigned gpu ) ;
      
      /** Method to retrieve the loaded character buffer of this object.
       * @return const reference to the loaded character buffer of this object.
       */
      inline const nyx::Array<Framework, nyx::NttFile::Character> characters() const ;
      
      /** Method to retrieve the texture array of this object.
       * @return The texture array of this object.
       */
      inline const std::vector<nyx::Image<Framework>>& textures() const ;

      /** Method to check whether this object has been initialized.
       * @return Whether or not this object has been initialized.
       */
      inline bool initialized() const ;
      
      /** Method to reset and deallocate all data.
       */
      inline void reset() ;
      
    private:
      void makeVertices() ;
      std::string                                      name               ;
      std::map<char, nyx::Array<Framework, glm::vec4>> character_vert_map ;
      nyx::Array<Framework, nyx::NttFile::Character>   d_characters       ;
      std::vector<nyx::NttFile::Character>             h_characters       ;
      std::vector<nyx::Image<Framework>>               d_textures         ;
  };
  
  template<typename Framework>
  Font<Framework>::Font()
  {
    this->d_textures = {} ;
    this->name       = "" ;
  }
  
  template<typename Framework>
  void Font<Framework>::initialize( const char* font_path, unsigned gpu )
  {
    nyx::NttFile                         file    ;
    nyx::Chain<Framework>                chain   ;
    nyx::Array<Framework, unsigned char> staging ;

    if( file.load( font_path ) )
    {
      chain             .initialize( gpu, nyx::ChainType::Compute ) ;
      staging           .initialize( gpu, 1024 * 1024             ) ;
      this->d_characters.initialize( gpu, file.characterCount()   ) ;
      
      this->d_textures  .resize( file.characterCount() ) ;
      this->h_characters.resize( file.characterCount() ) ;
      
      for( unsigned index = 0; index < file.characterCount(); index++ )
      {
        auto& character = file.character( static_cast<unsigned char>( index ) ) ;
        
        this->d_textures  [ index ].initialize( nyx::ImageFormat::R8, gpu, character.bearing.x, character.bearing.y ) ;
        this->h_characters[ index ] = character ;
        
        chain.copy( file.characterImage( static_cast<unsigned char>( index ) ), staging ) ;
        chain.copy( staging, this->d_textures[ index ]                                    ) ;
        chain.submit     () ;
        chain.synchronize() ;
      }
      
      chain.copy( this->h_characters.data(), this->d_characters ) ;
      chain.submit() ;
      chain.synchronize() ;
      
      chain.reset() ;
    }
    else
    {
      // TODO error.
    }
    
    file.reset() ;
  }
  
  template<typename Framework>
  void Font<Framework>::initialize( unsigned char* bytes, unsigned size, unsigned gpu )
  {
    nyx::NttFile                         file    ;
    nyx::Chain<Framework>                chain   ;
    nyx::Array<Framework, unsigned char> staging ;
    
    if( file.load( bytes, size ) )
    {
            chain             .initialize( gpu, nyx::ChainType::Compute ) ;
      staging           .initialize( gpu, 1024 * 1024             ) ;
      this->d_characters.initialize( gpu, file.characterCount()   ) ;
      
      this->d_textures  .resize( file.characterCount() ) ;
      this->h_characters.resize( file.characterCount() ) ;
      
      for( unsigned index = 0; index < file.characterCount(); index++ )
      {
        auto& character = file.character( static_cast<unsigned char>( index ) ) ;
        
        this->d_textures  [ index ].initialize( nyx::ImageFormat::R8, gpu, character.bearing.x, character.bearing.y ) ;
        this->h_characters[ index ] = character ;
        
        chain.copy( file.characterImage( static_cast<unsigned char>( index ) ), staging ) ;
        chain.copy( staging, this->d_textures[ index ]                                  ) ;
        chain.submit     () ;
        chain.synchronize() ;
      }
      
      chain.copy( this->h_characters.data(), this->d_characters ) ;
      chain.submit() ;
      chain.synchronize() ;
      
      chain.reset() ;
      
      this->makeVertices() ;
    }
    else
    {
      // TODO error.
    }
    file.reset() ;
  }
  
  template<typename Framework>
  void Font<Framework>::makeVertices()
  {
    for( auto& character : this->h_characters )
    {
//      this->character_vert_map[ character.]
    }
  }
  
  template<typename Framework>
  const nyx::Array<Framework, nyx::NttFile::Character> Font<Framework>::characters() const
  {
    return this->d_characters ;
  }
  
  template<typename Framework>
  const std::vector<nyx::Image<Framework>>& Font<Framework>::textures() const
  {
    return this->d_textures ;
  }
  
  template<typename Framework>
  bool Font<Framework>::initialized() const
  {
    return !this->d_textures.empty() ;
  }
}