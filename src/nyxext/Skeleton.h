/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Skeleton.h
 * Author: jhendl
 *
 * Created on May 16, 2021, 9:18 PM
 */

#pragma once
#include <NyxGPU/library/Array.h>
#include <NyxGPU/library/Chain.h>
#include <NyxGPU/library/Pipeline.h>
#include <NyxGPU/NssFile.h>
#include <glm/glm.hpp>
#include <vector>
#include <map>

namespace mars
{
  template<typename Framework>
  class Skeleton
  {
    public:
      Skeleton() ;
      ~Skeleton() ;
      void initialize( unsigned device, const char* nss_file_path ) ;
      bool initialized() const ;
      void setAnimation( unsigned animation ) ;
      void traverse( float delta_time ) ;
      void bind( nyx::Pipeline<Framework>& pipeline, const char* name ) ;
      void reset() ;
    private:
      using Bone      = nyx::NssFile::Bone      ;
      using Animation = nyx::NssFile::Animation ;
      
      void process( float delta_time, glm::mat4& transform, const Bone& bone, const Animation& animation ) ;
      glm::mat4 convert( nyx::NssFile::mat4& matrix ) const ;

      nyx::Array<Framework, glm::mat4> d_transforms ;
      
      unsigned               current_animation ;
      nyx::NssFile           bones             ;
      nyx::Chain<Framework>  chain             ;
      std::vector<glm::mat4> matrices          ;
      std::vector<Animation> animations        ;
  };
  
  template<typename Framework>
  Skeleton<Framework>::Skeleton()
  {
    ;
  }

  template<typename Framework>
  Skeleton<Framework>::~Skeleton()
  {
    ;
  }

  template<typename Framework>
  glm::mat4 Skeleton<Framework>::convert( nyx::NssFile::mat4& matrix ) const
  {
    glm::mat4 out ;
    
    out[ 0 ][ 0 ] = matrix.x.x ;
    out[ 0 ][ 1 ] = matrix.x.y ;
    out[ 0 ][ 2 ] = matrix.x.z ;
    out[ 0 ][ 3 ] = matrix.x.w ;
    out[ 1 ][ 0 ] = matrix.y.x ;
    out[ 1 ][ 1 ] = matrix.y.y ;
    out[ 1 ][ 2 ] = matrix.y.z ;
    out[ 1 ][ 3 ] = matrix.y.w ;
    out[ 2 ][ 0 ] = matrix.z.x ;
    out[ 2 ][ 1 ] = matrix.z.y ;
    out[ 2 ][ 2 ] = matrix.z.z ;
    out[ 2 ][ 3 ] = matrix.z.w ;
    out[ 3 ][ 0 ] = matrix.w.x ;
    out[ 3 ][ 1 ] = matrix.w.y ;
    out[ 3 ][ 2 ] = matrix.w.z ;
    out[ 3 ][ 3 ] = matrix.w.w ;
    
    return out ;
  }
  
  template<typename Framework>
  void Skeleton<Framework>::setAnimation( unsigned animation )
  {
    this->current_animation = animation ;
  }

  template<typename Framework>
  void Skeleton<Framework>::process( float delta_time, glm::mat4& transform, const Bone& bone, const Animation& animation )
  {
    glm::mat4   new_transform  ;
    std::string bone_name      ;
    std::string animation_name ;

    new_transform = glm::mat4( 1.0f ) ;
    bone_name     = bone.name()       ;

    for( unsigned index = 0; index < animation.nodeCount(); index++ )
    {
      animation_name = animation.name() ;
      if( animation_name == bone_name )
      {
        // Add animation transforms.
      }
    }
    
    new_transform = this->convert( bone.transform() ) * transform ;
    for( unsigned index = 0; index < bone.numChildren(); index++ )
    {
      process( new_transform, bone.child( index ), animation ) ;
    }
  }
  
  template<typename Framework>
  void Skeleton<Framework>::traverse( float delta_time )
  {
    const auto& bone = this->bones.root() ;
    glm::mat4 transform ;
    
    transform = glm::mat4( 1.0f ) ;
    
    this->process( delta_time, transform, bone, this->bones.animation( this->current_animation ) ) ;
  }

  template<typename Framework>
  void Skeleton<Framework>::initialize( unsigned device, const char* nss_file_path )
  {
    if( this->bones.load( nss_file_path ) )
    {
      this->chain.initialize( device, nyx::ChainType::Compute ) ;
      this->matrices.resize( 100 ) ;
      this->d_transforms.initialize( device, 100, false, nyx::ArrayFlags::UniformBuffer ) ;
      this->traverse() ;
    }
  }

  template<typename Framework>
  bool Skeleton<Framework>::initialized() const
  {
    return this->d_transforms.initialized() ;
  }

  template<typename Framework>
  void Skeleton<Framework>::bind( nyx::Pipeline<Framework>& pipeline, const char* name )
  {
    pipeline.bind( name, this->d_transforms ) ;
  }

  template<typename Framework>
  void Skeleton<Framework>::reset()
  {
    this->d_transforms.reset() ;
  }
}