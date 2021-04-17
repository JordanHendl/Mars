# Mars
  Mars is a template library designed to provide functionality for game/graphic engine development.

## How to build and install
  Mars is built using **CMake**. If on linux, to build from source, simply do: 
  
  ```
  mkdir build
  cd build
  cmake ..
  make 
  ```
  
  For Windows, do 
  
  ```
  mkdir build
  cd build
  cmake ..
  ```
  
  And then simply open the generated .sln with whatever editor of your preference and build.

  Generated packages's default install to '/usr/local/lib/Mars' on UNIX, and 'C:\Program Files\Mars' on Windows.

  Using with CMake: 
  1) Add the path to the install to your *CMAKE_PREFIX_PATH*.
  2) ```FIND_PACKAGE( Mars ) ```
  3) Link against any Mars library ( mars, mars_nyxext, etc. ) you need!
  
## Dependancies

  Base, Mars depends on nothing. However, if it can find the libraries on the system, it will build what it can. 
 
  E.g. If NyxGPU library is found, the Nyx Extention headers will be built.

  Athena is used for testing. To output tests, install the athena testing library. ( See https://github.com/JordanHendl/athena )

## Closing
  If you have any questions or suggestions shoot me an email at jordiehendl@gmail.com
  or hit me up on twitter ( twitter.com/jajajordie )
