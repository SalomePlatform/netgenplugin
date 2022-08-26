// Copyright (C) 2007-2021  CEA/DEN, EDF R&D, OPEN CASCADE
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
//
// See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
//

// File      : NETGENPlugin_Provider.hxx
// Author    : Yoann AUDOUIN (EDF)
// Project   : SALOME
//
#ifndef _NETGENPlugin_Provider_HXX_
#define _NETGENPlugin_Provider_HXX_

#include <iostream>
#include <thread>
#include <array>
#include <vector>
#include <unistd.h>
#include <mutex>

namespace nglib {
#include <nglib.h>
}
#ifndef OCCGEOMETRY
#define OCCGEOMETRY
#endif
#include <occgeom.hpp>
#include <meshing.hpp>

template<class T, int NDATA>
class ProviderPtr{
  public:

    ProviderPtr(){
      for(int i=0;i<NDATA;i++){
        this->_mydata[i] = nullptr;
        this->_useddata[i] = false;
      }
    }

    int take(T** data){

      this->_mymutex.lock();
      *data = nullptr;
      for(int i=0;i<NDATA;i++){
        if(!this->_useddata[i]){
          if (this->_mydata[i] == nullptr)
            this->_mydata[i] = new T();
          this->_useddata[i] = true;
          *data = this->_mydata[i];
          this->_mymutex.unlock();
          return i;
        }
      }
      this->_mymutex.unlock();
      return -1;
    };


    bool release(int i, bool clean){

      this->_mymutex.lock();

      if(clean){
        delete this->_mydata[i];
        this->_mydata[i] = nullptr;
      }

      this->_useddata[i] = false;

      this->_mymutex.unlock();

      return true;
    };

    void dump(){
      std::cout << "Dumping provider:" << std::endl;
      for(int i=0;i<NDATA;i++){
        std::cout << " - " << i << " used: " << this->_useddata[i] << std::endl;
        std::cout << " - adress: " << this->_mydata[i] << std::endl;
        if (this->_mydata[i] != nullptr)
          std::cout << " -  i: " << this->_mydata[i]->i << " d: " << this->_mydata[i]->d << std::endl;
      }
    };

  private:
    std::array<T*, NDATA> _mydata;
    std::array<bool, NDATA> _useddata;
    std::mutex _mymutex;

};

template<class T, int NDATA>
class Provider{
  public:

    Provider() = default;

    int take(T& data){

      this->_mymutex.lock();
      for(int i=0;i<NDATA;i++){
        if(!this->_useddata[i]){
          this->_useddata[i] = true;
          data = this->_mydata[i];
          this->_mymutex.unlock();
          return i;
        }
      }
      this->_mymutex.unlock();
      return -1;
    };


    bool release(int i){

      this->_mymutex.lock();
      this->_useddata[i] = false;
      this->_mymutex.unlock();

      return true;
    };

    void dump(){
      std::cout << "Dumping provider:" << std::endl;
      for(int i=0;i<NDATA;i++){
        std::cout << " - " << i << " used: " << this->_useddata[i] << std::endl;
        std::cout << " -  i: " << this->_mydata[i].i << " d: " << this->_mydata[i].d << std::endl;
      }
    };

  private:
    std::array<T, NDATA> _mydata;
    std::array<bool, NDATA> _useddata;
    std::mutex _mymutex;

};

#endif