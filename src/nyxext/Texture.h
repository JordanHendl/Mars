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
 * File:   Texture.h
 * Author: jhendl
 *
 * Created on April 25, 2021, 11:53 PM
 */

#pragma once
#include "NyxGPU/NgtFile.h"
#include "NyxGPU/library/Array.h"
#include "NyxGPU/library/Image.h"
#include "NyxGPU/library/Chain.h"
#include <vector>
#include <string>
#include <map>

namespace mars
{
  /** Template texture class. Acts as a 3D texture, and provides functionality for drawing.
   */
  template<typename Framework>
  class Texture
  {
    public:
      
      /** Constructor.
       */
      Texture() ;
      
      /** Method to retrieve the underlying image of this object.
       * @return The underlying gpu image of this object.
       */
      inline operator nyx::Image<Framework>&() ;
      
      /** Method to initialize this object.
       * @param texture_path The path to the .ngt file on disk to load.
       * @param gpu the gpu to allocate the texture on.
       */
      inline void initialize( const char* ngt_path, unsigned gpu ) ;
      
      /** Method to initialize this object.
       * @param bytes The bytes containing the .ngt file.
       * @param size  The size of the input bytes.
       * @param gpu the gpu to allocate the texture on.
       */
      inline void initialize( const unsigned char* bytes, unsigned size, unsigned gpu ) ;
      
      /** Method to initialize this object.
       * @param file The file with the preloaded texture on it.
       * @param gpu The gpu to allocate the texture on.
       */
      inline void initialize( nyx::NgtFile& file, unsigned gpu ) ;
      
      /** Method to retrieve a pointer to this object's internal image.
       * @return Const pointer to this object's internal image.
       */
      inline const nyx::Image<Framework>* pointer() const ;

      /** Method to check whether this object has been initialized.
       * @return Whether or not this object has been initialized.
       */
      inline bool initialized() const ;
      
      /** Method to reset and deallocate all data.
       */
      inline void reset() ;
      
    private:
      nyx::Image<Framework> image ;
  };
  
  template<typename Framework>
  Texture<Framework>::Texture()
  {
  }
  
  template<typename Framework>
  Texture<Framework>::operator nyx::Image<Framework>&()
  {
    return this->image ;
  }

  template<typename Framework>
  void Texture<Framework>::initialize( const char* texture_path, unsigned gpu )
  {
    nyx::NgtFile                         file    ;
    nyx::Chain<Framework>                chain   ;
    nyx::Array<Framework, unsigned char> staging ;

    file.load( texture_path ) ;
    
    if( file.width() != 0 && file.height() != 0 )
    {
      this->image.initialize( nyx::ImageFormat::RGBA8, gpu, file.width(), file.height() ) ;
      staging    .initialize( gpu, file.width() * file.height() * file.channels()       ) ;
      chain      .initialize( gpu, nyx::ChainType::Compute                              ) ;
      
      chain.copy         ( file.image(), staging ) ;
      chain.memoryBarrier( staging, this->image  ) ;
      chain.copy         ( staging, this->image  ) ;
      chain.submit() ;
      chain.synchronize() ;
      
      chain.transition( this->image, nyx::ImageLayout::ShaderRead ) ;
      chain.submit() ;
      chain.synchronize() ;
      staging.reset() ;
      chain  .reset() ;
    }
  }
  
  template<typename Framework>
  void Texture<Framework>::initialize( const unsigned char* bytes, unsigned size, unsigned gpu )
  {
    nyx::NgtFile                         file    ;
    nyx::Chain<Framework>                chain   ;
    nyx::Array<Framework, unsigned char> staging ;

    file.load( bytes, size ) ;
    
    if( file.width() != 0 && file.height() != 0 )
    {
      this->image.initialize( nyx::ImageFormat::RGBA8, gpu, file.width(), file.height() ) ;
      staging    .initialize( gpu, file.width() * file.height() * file.channels()       ) ;
      chain      .initialize( gpu, nyx::ChainType::Compute                              ) ;
      
      chain.copy         ( file.image(), staging ) ;
      chain.memoryBarrier( staging, this->image  ) ;
      chain.copy         ( staging, this->image  ) ;
      
      chain.submit() ;
      chain.synchronize() ;
      
      chain.transition( this->image, nyx::ImageLayout::ShaderRead ) ;
      chain.submit() ;
      chain.synchronize() ;
      
      staging.reset() ;
      chain  .reset() ;
    }
  }
  
  template<typename Framework>
  void Texture<Framework>::initialize( nyx::NgtFile& file, unsigned gpu )
  {
    nyx::Chain<Framework>                chain   ;
    nyx::Array<Framework, unsigned char> staging ;

    if( file.width() != 0 && file.height() != 0 )
    {
      this->image.initialize( nyx::ImageFormat::RGBA8, gpu, file.width(), file.height() ) ;
      staging    .initialize( gpu, file.width() * file.height() * file.channels()       ) ;
      chain      .initialize( gpu, nyx::ChainType::Compute                              ) ;
      
      chain.copy         ( file.image(), staging ) ;
      chain.memoryBarrier( staging, this->image  ) ;
      chain.copy         ( staging, this->image  ) ;
      chain.submit() ;
      chain.synchronize() ;
      
      chain.transition( this->image, nyx::ImageLayout::ShaderRead ) ;
      chain.submit() ;
      chain.synchronize() ;
      staging.reset() ;
      chain  .reset() ;
    }
  }
  
  template<typename Framework>
  const nyx::Image<Framework>*  Texture<Framework>::pointer() const
  {
    return &this->image ;
  }
  
  template<typename Framework>
  bool Texture<Framework>::initialized() const
  {
    return this->image.initialized() ;
  }
}