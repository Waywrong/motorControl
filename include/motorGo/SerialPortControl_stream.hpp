/*
//First edition by old000: C.H.Hung , NTU,EE97,master student
//Second edition by kenji: Y.C.Lin , NTU,EE97,master student
//          Idea from code by W.L.Hsu, NTU,EE98,PHD student

// MOUAHAHHAHA MALCOLM IS THERE ! :D
*/
#ifndef _SERIALPORTCONTROL_HPP
#define _SERIALPORTCONTROL_HPP
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <deque>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <string>
#include <stdexcept>
#include <SerialStream.h>
#include <boost/lexical_cast.hpp>
#include "ros/ros.h"

int testin;

//That must be parameters !!!
//#define TICKNUM 500000
//#define PORT "/dev/ttyUSB0"

class SerialPortControl{
  protected :
  //ATTRIBUTS 
  LibSerial::SerialStream _motor;
  int _lEnco;
  int _rEnco;
  int _liftEnco;
  float _targetSRW; //Speed in rpm
  float _targetSLW;
  float _targetSLift;
  float _measuredSRW; //Speed of right wheel mesured
  float _measuredSLW;
  float _measuredSLift;
  double _nTick;
  double _nTickLift;
  std::string _LWHEEL;
  std::string _RWHEEL;
  std::string _LIFTER;
  bool _verbose;
  bool _readcorrectly;
  ros::NodeHandle _pnode;
  
  public:
  SerialPortControl(int maxspeed, ros::NodeHandle priv_node) : _lEnco(0),_rEnco(0),_targetSRW(0),_targetSLW(0),_measuredSRW(0),_measuredSLW(0), _verbose(false), _readcorrectly(true), _pnode(priv_node){
    //ros::param::get("/TobotDriver/NodeleftWheel", _LWHEEL);
    //ros::param::get("/TobotDriver/NodeRightWheel", _RWHEEL);
    //ros::param::get("/TobotDriver/TICKNUM", _nTick);
    _pnode.param<std::string>("NodeleftWheel", _LWHEEL, "1");
    _pnode.param<std::string>("NodeleftRheel", _RWHEEL, "2");
    _pnode.param<std::string>("NodeLifter", _LIFTER, "3");
    _pnode.param<double>("TICKNUM", _nTick, 600000);
    _pnode.param<double>("TICKNUMLIFT", _nTickLift, 500000);
    //_LWHEEL="1";
    //_RWHEEL="1";
    //std::string PORT ="/dev/ttyUSB0";
    std::string PORT;
    _pnode.param<std::string>("Port", PORT, "/dev/ttyUSB0");
    double Baud;
    //ros::param::get("/TobotDriver/BaudRate", Baud);
    _pnode.param<double>("BaudRate", Baud, 9600);
    std::cout << "The motor is=> Baud "<< Baud<<" Port "<< PORT<<std::endl;
    _motor.Open(PORT);		
    //8 data bits
    //1 stop bit
    //No parity
      if ( ! _motor.good() ){
	    std::cerr << "[" << __FILE__ << ":" << __LINE__ << "] "
	    << "Error: Could not open serial port "<<PORT
	    << std::endl ;
	    exit(1) ;
    }
	  
    if(Baud==115200){
      _motor.SetBaudRate(LibSerial::SerialStreamBuf::BAUD_115200);
      std::cout<<"Going for value of 115200"<<std::endl;
    }
    else if(Baud==1200){
      _motor.SetBaudRate(LibSerial::SerialStreamBuf::BAUD_1200);
      std::cout<<"Going for value of 1200"<<std::endl;
    }
    else if(Baud==2400){
      _motor.SetBaudRate(LibSerial::SerialStreamBuf::BAUD_2400);
      std::cout<<"Going for value of 2400"<<std::endl;
    }
    else if(Baud==19200){
      _motor.SetBaudRate(LibSerial::SerialStreamBuf::BAUD_19200);
      std::cout<<"Going for value of 19200"<<std::endl;
    }
    else if(Baud==38400){
      _motor.SetBaudRate(LibSerial::SerialStreamBuf::BAUD_38400);
      std::cout<<"Going for value of 38400"<<std::endl;
    }
    else if(Baud==57600){
      _motor.SetBaudRate(LibSerial::SerialStreamBuf::BAUD_57600);
      std::cout<<"Going for value of 57600"<<std::endl;
    }
    else if(Baud==9600){
      _motor.SetBaudRate(LibSerial::SerialStreamBuf::BAUD_9600);
      std::cout<<"Going for default value of 9600"<<std::endl;
    }
    else{
      std::cout<<"Value incorrect. Going for default value of 9600"<<std::endl;
      _motor.SetBaudRate(LibSerial::SerialStreamBuf::BAUD_9600);
    }
	      
    if ( ! _motor.good() ){
      std::cerr << "Error: Could not set the baud rate." <<std::endl;
      exit(1);
    }		
    
    
    _motor.SetCharSize(LibSerial::SerialStreamBuf::CHAR_SIZE_8);
    if ( ! _motor.good() ){
      std::cerr << "Error: Could not set the char size." <<std::endl;
      exit(1);
    }
    _motor.SetParity( LibSerial::SerialStreamBuf::PARITY_NONE ) ;
    if ( ! _motor.good() ){
      std::cerr << "Error: Could not set the parity." <<std::endl;
      exit(1);
    }
    _motor.SetFlowControl(LibSerial::SerialStreamBuf::FLOW_CONTROL_NONE ) ;
    if ( ! _motor.good() ){
      std::cerr << "Error: Could not set the control." <<std::endl;
      exit(1);
    }
    _motor.SetNumOfStopBits(1) ;
    if ( ! _motor.good() ){
      std::cerr << "Error: Could not set the stopbit." <<std::endl;
      exit(1);
    }
    

    //setMax Min speed
    //writeDisable(); //Stop the motor in case the robot was still moving

    writeEnable();
    std::string s("V0\r");
    writePort(s); //to stop the motor

    

    writeMaxSpeed(maxspeed);

    writeMinSpeed(-maxspeed);
    writeHome(); //Set the initial position to zero ;)


  }
  
  ~SerialPortControl(){
	  _motor.Close();
  }
	
  /**************************************************
  Accessors
  **************************************************/
  int getLencoder(){return _lEnco;}
  int getRencoder(){return _rEnco;}
  int getLiftencoder(){return _liftEnco;}
  float getTargetSLW(){return _targetSLW;}
  float getTargetSRW(){return _targetSRW;}
  float getTargetSLift(){return _targetSLift;}
  float getMeasuredSLW(){return _measuredSLW;}
  float getMeasuredSRW(){return _measuredSRW;}
  float getMeasuredSLift(){return _measuredSLift;}
  LibSerial::SerialStream& getMotor(){return _motor;}
  double getTickNumber(){return _nTick;}
  double getTickNumberLift(){return _nTickLift;}
  bool getReadState(){return _readcorrectly;}
  /**************************************************
  Read values in the controller
  **************************************************/
  int readLencoder(); //Left wheel node 0
  int readRencoder(); //Right wheel node 1
  int readLiftencoder(); //Lift motor node 3
  int readTargetSLW();
  int readTargetSRW();
  int readTargetSLift();
  int readRealSRW();
  int readRealSLW();
  int readRealSLift();
  int readEncoderResolution();
  int readEncoderResolutionLift();

  /**************************************************
  Set values Manually
  **************************************************/	
  void setTargetSLW(float s){_targetSLW=s;}
  void setTargetSRW(float s){_targetSRW=s;}
  void setTargetSLift(float s){_targetSLift=s;}
  void setVerbose(){_verbose=true;}
  void setReadState(){_readcorrectly=true;}
  void setTickNumber(int n){_nTick=n;}
  void setTickNumberLift(int n){_nTickLift=n;}
  
  /**************************************************
  Write values in the controller
  **************************************************/
  void writeEnable();
  void writeDisable();
  void writeGoEncoderIndex();
  void writeHome();
  void writeAcc(int acc);
  void writeDec(int dec);
  void writeHomeLifter();
  void writeMaxSpeed(int ms){
    try{
      std::string MaxSpeed=boost::lexical_cast<std::string>(ms);
      MaxSpeed="SP"+MaxSpeed+"\n";
      writePort(MaxSpeed);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Max Speed" << blc.what() << std::endl;
      _readcorrectly=false;
//	  scanf("%d",&testin);
    }
  }
  void writeMinSpeed(int ms){
    try{
      std::string MaxSpeed=boost::lexical_cast<std::string>(ms);
      MaxSpeed="MV"+MaxSpeed+"\n";
      writePort(MaxSpeed);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Min Speed" << blc.what() << std::endl;
//	  scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  void writeSpeed(int ms){
    try{
      std::string MaxSpeed=boost::lexical_cast<std::string>(ms);
      MaxSpeed="V"+MaxSpeed+"\n";
      writePort(MaxSpeed);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Speed" << blc.what() << std::endl;
//	  scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  void writeTargetSRW(int ms){
    try{
      std::string MaxSpeed=boost::lexical_cast<std::string>(ms);
      MaxSpeed=_RWHEEL+"V"+MaxSpeed+"\n";
      writePort(MaxSpeed);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Right wheel Speed" << blc.what() << std::endl;
//	  scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  void writeTargetSLW(int ms){
    try{
      std::cout<<"we write ";
      std::string MaxSpeed=boost::lexical_cast<std::string>(ms);
      MaxSpeed=_LWHEEL+"V"+MaxSpeed+"\n";
      std::cout<<MaxSpeed<<std::endl;
      writePort(MaxSpeed);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Left wheel Speed" << blc.what() << std::endl;
//	  scanf("%d",&testin);
      _readcorrectly=false;
    }	
  }
  void writeTargetSLift(int ms){
    try{
      std::cout<<"we write ";
      std::string MaxSpeed=boost::lexical_cast<std::string>(ms);
      MaxSpeed=_LIFTER+"V"+MaxSpeed+"\n";
      std::cout<<MaxSpeed<<std::endl;
      writePort(MaxSpeed);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Left wheel Speed" << blc.what() << std::endl;
//	  scanf("%d",&testin);
      _readcorrectly=false;
    }	
  }
  void writePoseRelativeR(int ms){
    try{
      std::string pos=boost::lexical_cast<std::string>(ms);
      pos=_RWHEEL+"LR"+pos+"\n";
      writePort(pos);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Relative Right position" << blc.what() << std::endl;
      // scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  void writePoseRelativeL(int ms){
    try{
      std::string pos=boost::lexical_cast<std::string>(ms);
      pos=_LWHEEL+"LR"+pos+"\n";
      writePort(pos);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Relative Left position" << blc.what() << std::endl;
      // scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  void writePoseRelativeLifter(int ms){
    try{
      std::string pos=boost::lexical_cast<std::string>(ms);
      pos=_LIFTER+"LR"+pos+"\n";
      writePort(pos);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Relative Left position" << blc.what() << std::endl;
      // scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  void writePoseAbsoluteR(int ms){
    try{
      std::string pos=boost::lexical_cast<std::string>(ms);
      pos=_RWHEEL+"LA"+pos+"\n";
      writePort(pos);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Absolute Right Position" << blc.what() << std::endl;
      //scanf("%d",&testin);
      _readcorrectly=false;
    }

  }
  void writePoseAbsoluteL(int ms){
    try{
      std::string pos=boost::lexical_cast<std::string>(ms);
      pos=_LWHEEL+"LA"+pos+"\n";
      writePort(pos);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Absolute Left Position" << blc.what() << std::endl;
      //scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  void writePoseAbsoluteLifter(int ms){
    try{
      std::string pos=boost::lexical_cast<std::string>(ms);
      pos=_LIFTER+"LA"+pos+"\n";
      writePort(pos);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in Absolute Left Position" << blc.what() << std::endl;
      //scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  void writeMoveLeftWheel(){
    std::string s=_LWHEEL+"M\n";
    writePort(s);
  }
  void writeMoveRightWheel(){
    std::string s=_RWHEEL+"M\n";
    writePort(s);
  }
  void writeMoveLifter(){
    std::string s=_LIFTER+"M\n";
    writePort(s);
  }
  /**************************************************
  Basic serial operations
  **************************************************/
  
  void emptyBuffer();
  void writePort(std::string& str);
  std::string readPort();

  /**************************************************
  Updates
  **************************************************/	
  void update(int speedRwheel, int speedLwheel);
};

/*********************************************************************************************************
FONCTIONS
*********************************************************************************************************/

/********************************Read values in the controller**************************************/

inline int SerialPortControl::readLencoder(){
  emptyBuffer();
  std::string yo("POS\n");
  yo=_LWHEEL+yo;
  writePort(yo);
  std::string result=readPort();
  if (strcmp("fail", result.c_str() )!= 0){
    try{
      return boost::lexical_cast<int>(result);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in readPort" << blc.what()<< " this is the reason " <<result<< " VOILA" << std::endl;
      // scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  else{
    return 0;
    _readcorrectly=false;
  }
}

inline int SerialPortControl::readRencoder(){
	emptyBuffer();
	std::string yo("POS\n");
	yo=_RWHEEL+yo;
	writePort(yo);
	std::string result=readPort();
	if (strcmp("fail", result.c_str())!= 0){
	  try{
	    return boost::lexical_cast<int>(result);
	  }
	  catch(boost::bad_lexical_cast& blc){
	    std::cout << "Exception in readPort" << blc.what()<< " this is the reason " <<result<< " VOILA" << std::endl;
	    // scanf("%d",&testin);
	    _readcorrectly=false;
	  }
	}
	else{
	  return 0;
	  _readcorrectly=false;
	}
}

inline int SerialPortControl::readLiftencoder(){
	emptyBuffer();
	std::string yo("POS\n");
	yo=_LIFTER+yo;
	writePort(yo);
	std::string result=readPort();
	if (strcmp("fail", result.c_str())!= 0){
	  try{
		  return boost::lexical_cast<int>(result);
	  }
	  catch(boost::bad_lexical_cast& blc){
	    std::cout << "Exception in readPort" << blc.what()<< " this is the reason " <<result<< " VOILA" << std::endl;
	    // scanf("%d",&testin);
	    _readcorrectly=false;
	  }
	}
	else{
	  return 0;
	  _readcorrectly=false;
	}
}

inline int SerialPortControl::readEncoderResolution(){
	emptyBuffer();
	std::string yo("GENCRES\n");
	yo=_LWHEEL+yo;
	writePort(yo);
	std::string result=readPort();
	if (strcmp("fail", result.c_str())!= 0){
	  try{
	    return boost::lexical_cast<int>(result);
	  }
	  catch(boost::bad_lexical_cast& blc){
	    std::cout << "Exception in readPort" << blc.what()<< " this is the reason " <<result<< " VOILA" << std::endl;
	    // scanf("%d",&testin);
	    _readcorrectly=false;
	  }
	}
	else{
	  return 0;
	  _readcorrectly=false;
	}
	
}

inline int SerialPortControl::readEncoderResolutionLift(){
  emptyBuffer();
  std::string yo("GENCRES\n");
  yo=_LIFTER+yo;
  writePort(yo);
  std::string result=readPort();
  if (strcmp("fail", result.c_str())!= 0){
    try{
      return boost::lexical_cast<int>(result);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in readPort" << blc.what()<< " this is the reason " <<result<< " VOILA" << std::endl;
      // scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  else{
    return 0;
    _readcorrectly=false;
  }
	
}


inline int SerialPortControl::readTargetSRW(){
  emptyBuffer();
  std::string speed2("GV\n");
  speed2=_RWHEEL+speed2;
  writePort(speed2);
  std::string result=readPort();
  if (strcmp("fail", result.c_str())!= 0){
    try{
	    return boost::lexical_cast<int>(result);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in readPort" << blc.what()<< " this is the reason " <<result<< " VOILA" << std::endl;
      // scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  else{
    return 0;
    _readcorrectly=false;
  }
}

inline int SerialPortControl::readTargetSLW(){
  emptyBuffer();
  std::string speed2("GV\n");
  speed2=_LWHEEL+speed2;
  writePort(speed2);
  std::string result=readPort();
  if (strcmp("fail", result.c_str())!= 0){
    try{
	    return boost::lexical_cast<int>(result);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in readPort" << blc.what()<< " this is the reason " <<result<< " VOILA" << std::endl;
      // scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  else{
    return 0;
    _readcorrectly=false;
  }
}

inline int SerialPortControl::readTargetSLift(){
  emptyBuffer();
  std::string speed2("GV\n");
  speed2=_LIFTER+speed2;
  writePort(speed2);
  std::string result=readPort();
  if (strcmp("fail", result.c_str())!= 0){
    try{
	    return boost::lexical_cast<int>(result);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in readPort" << blc.what()<< " this is the reason " <<result<< " VOILA" << std::endl;
      // scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  else{
    return 0;
    _readcorrectly=false;
  }		
}


inline int SerialPortControl::readRealSRW(){	
  emptyBuffer();
  std::string speed2("GN\n");
  speed2=_RWHEEL+speed2;
  writePort(speed2);
  std::string result=readPort();
  if (strcmp("fail", result.c_str())!= 0){
    try{
	    return boost::lexical_cast<int>(result);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in readPort" << blc.what()<< " this is the reason " <<result<< " VOILA" << std::endl;
      // scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  else{
    return 0;
    _readcorrectly=false;
  }
}

inline int SerialPortControl::readRealSLW(){	
  emptyBuffer();
  std::string speed2("GN\n");
  speed2=_LWHEEL+speed2;
  writePort(speed2);
  std::string result=readPort();
  if (strcmp("fail", result.c_str())!= 0){
    try{
	    return boost::lexical_cast<int>(result);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in readPort" << blc.what()<< " this is the reason " <<result<< " VOILA" << std::endl;
      // scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  else{
    return 0;
  }

}

inline int SerialPortControl::readRealSLift(){	
  emptyBuffer();
  std::string speed2("GN\n");
  speed2=_LIFTER+speed2;
  writePort(speed2);
  std::string result=readPort();
  if (strcmp("fail", result.c_str())!= 0){
    try{
	    return boost::lexical_cast<int>(result);
    }
    catch(boost::bad_lexical_cast& blc){
      std::cout << "Exception in readPort" << blc.what()<< " this is the reason " <<result<< " VOILA" << std::endl;
      // scanf("%d",&testin);
      _readcorrectly=false;
    }
  }
  else{
    return 0;
  }

}

/*****************************CFULHABER CONTROLER CONFIGURATION*******************************/

inline void SerialPortControl::writeEnable(){
  std::string enable("1en\n2en\n");
  SerialPortControl::writePort(enable);
  //std::cout << "did we succesed : " << readPort() <<std::endl;
}

inline void SerialPortControl::writeDisable(){
  std::string enable("di\n");
  SerialPortControl::writePort(enable);
  //std::cout << "did we succesed : " << readPort() <<std::endl;
}

inline void SerialPortControl::writeGoEncoderIndex(){
  std::string enable("GOIX\n");
  SerialPortControl::writePort(enable);
  //std::cout << "did we succesed : " << readPort() <<std::endl;
}
inline void SerialPortControl::writeHome(){
  std::string enable("HO\n");
  SerialPortControl::writePort(enable);
  //std::cout << "did we succesed : " << readPort() <<std::endl;
}
inline void SerialPortControl::writeAcc(int acc){
  std::string accc=boost::lexical_cast<std::string>(acc);
  std::string enable("AC"+accc+"\n");
  SerialPortControl::writePort(enable);
  //std::cout << "did we succesed : " << readPort() <<std::endl;
}
inline void SerialPortControl::writeDec(int dec){
  std::string decc=boost::lexical_cast<std::string>(dec);
  std::string enable("DEC"+decc+"\n");
  SerialPortControl::writePort(enable);
  //std::cout << "did we succesed : " << readPort() <<std::endl;
}


/******************FOR THE ROBOT DIRECTLY****************/
inline void SerialPortControl::writeHomeLifter(){
  std::string enable("HO\n");
  enable=_LIFTER+enable;
  SerialPortControl::writePort(enable);
}

/****************************Basic functon*******************************************/

inline void SerialPortControl::writePort(std::string& str){
  _motor << str;//lpBufferToWrite;
  std::cout << "wrote the demand "<<str.c_str()<<" written " << std::endl;
  if(_verbose==true){
	  //std::cout << "wrote the demand "<<str.c_str()<<" written " << std::endl;
  }
}

inline std::string SerialPortControl::readPort(){
  std::string i("");
  if(_verbose==true){
    while(_motor.rdbuf()->in_avail() <=0){
      //std::cout<<"waiting for something to read"<<std::endl;
    }
    if(_motor.rdbuf()->in_avail() >0){	
	    
      /*char next_byte;
      _motor.get(next_byte);
      std::cerr << "the byte "<< next_byte<<std::endl;
      
      i=next_byte;
      std::string buf;
      buf=next_byte;
      

      while(strcmp("\n" , buf.c_str() ) != 0){

	_motor.get(next_byte);  //HERE I RECEIVE THE FIRST ANSWER
	//std::cerr << "the byte "<< next_byte << " with "<<str.compare(&next_byte) <<std::endl;
	i+=next_byte;
	buf=next_byte;
	std::cout<< "In the end "<< i << " and "<<buf<< " done "<<std::endl;
	}
      std::cout<<"OUT"<<std::endl;
      */
      
      _motor >> i;
      if(i.size()==0){i="fail";_readcorrectly=false;}
      std::cout<<"we read "<<i<<std::endl;
      return i;
    }
  }
  else{
    while(_motor.rdbuf()->in_avail() <=0){
    }
      if(_motor.rdbuf()->in_avail() >0){
	      
	/*char next_byte;
	_motor.get(next_byte);
	std::cerr << "the byte "<< next_byte<<std::endl;
	
	i=next_byte;
	std::string buf;
	buf=next_byte;
	

	while(strcmp("\n" , buf.c_str() ) != 0){

		_motor.get(next_byte);  //HERE I RECEIVE THE FIRST ANSWER
		//std::cerr << "the byte "<< next_byte << " with "<<str.compare(&next_byte) <<std::endl;
		i+=next_byte;
		buf=next_byte;
		std::cout<< "In the end "<< i << " and "<<buf<< " done "<<std::endl;
		}
	std::cout<<"OUT"<<std::endl;
	*/
	
	_motor >> i;
	if(i.size()==0){i="fail";_readcorrectly=false;}
	std::cout<<"we read "<<i<<std::endl;
	return i;

    }
  }
  std::cout<<"nothing to read"<<std::endl;
  _readcorrectly=false;
}

inline void SerialPortControl::emptyBuffer(){
  std::cout<<"emptying the buffer"<<std::endl;
  while(_motor.rdbuf()->in_avail() >0){
    char next_byte;
    _motor.get(next_byte);
    std::cerr << next_byte;
  }
  std::cout<<"OUT"<<std::endl;
		      
}

/**************************Mise à jour des vitesse*******************************************/
/*Speeds must be in rpm at that point*/
inline void SerialPortControl::update(int speedRwheel, int speedLwheel){
  if(_verbose==true){
    std::cout<<"ON FAIT DES TRUCS COOLS AUX MOTEURS..."<<std::endl;
  }
  _targetSRW=speedRwheel;
  _targetSLW=speedLwheel;	
  
  //TO REMOVE
  //writeSpeed(speedRwheel);
  //Write the target speeds
  
  writeTargetSRW(speedRwheel);
  writeTargetSLW(speedLwheel);
      
}


#endif
