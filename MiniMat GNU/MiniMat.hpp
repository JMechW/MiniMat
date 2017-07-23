/*
*File Name: MiniMat.hpp
*File Creation Date: April 17th 2017
*File Final Revised Date: May 7th 2017
*Author: Jiamin Wang
*Description: Header of MiniMat
*/

/*
*About:
*MiniMat is the Matrix Calculation Source Code especially designed for the Embedded System Development.
*The Source Code is inspired by Eigen (http://eigen.tuxfamily.org/)
*MiniMat currently only support fundamental calculations of Matrixes.
*MiniMat currently does not support typo or error checks in matrix calculations.
*MiniMat has not yet been fully tested, so bugs may still exist. Use with caution!
*/

/*
*Note:
*The matrix definitions in MiniMat is very strict which requires fixed size and types.
*The feedback of matrix relies on Heap Memory Allocations.
*All of the "friend" and "operator overloading" functions will destroy the input object if is temporary.
*MiniMat_HeapScope can be used to monitor heap status when activated.
*If defined for _MICROCHIP, iostream and some type definitions will be disabled;
*DO NOT ATTEMPT RISKY OPERATIONS SINCE THEY MAY CAUSE UNEXPECTED ERRORs!!!
*For examples, please check Main.cpp.
*/

#ifdef __JRB
#ifndef _JRB_MINIMAT_HPP_
#define _JRB_MINIMAT_HPP_

#include <stdlib.h>
#include <stdint.h>
#include <cmath>
#include <typeinfo>

#ifdef OS_USE_SEMIHOSTING
	#include <iostream>
#endif

#ifndef __HEAPSCOPE
	#define __HEAPSCOPE 1
#endif

#ifndef __SIMPLETYPE
	#define __SIMPLETYPE
	typedef uint8_t u8;
	typedef uint16_t u16;
	typedef uint32_t u32;
	typedef int8_t s8;
	typedef int16_t s16;
	typedef int32_t s32;
#endif

#if __HEAPSCOPE
	class _Mat_Heap
	{
	public:
		u32 _CurrentByte;//Current Heap Usage (Bytes)
		u32 _MaxByte;//Peak of Heap Usage
		u32 _CurrentTempNum;//Current Temporary Matrix Usage
		u32 _MaxTempNum;//Peak of Temporary Matrix Usage

	public:
		explicit _Mat_Heap():
		_CurrentByte(0),_MaxByte(0),_CurrentTempNum(0),_MaxTempNum(0)
		{};
		~_Mat_Heap(){};

		_Mat_Heap _HeapPlus(u32 plusnum)
		{
			_CurrentTempNum++;
			_MaxTempNum=((_CurrentTempNum>_MaxTempNum)?_CurrentTempNum:_MaxTempNum);

			_CurrentByte+=plusnum;
			_MaxByte=((_CurrentByte>_MaxByte)?_CurrentByte:_MaxByte);

			return *this;
		}

		_Mat_Heap _HeapMinus(u32 minusnum)
		{
			_CurrentTempNum--;
			_CurrentByte-=minusnum;

			return *this;
		}

#ifdef OS_USE_SEMIHOSTING
		void Print()
		{
			std::cout<<std::endl;
			std::cout<<"Heap Memory Usage Report:"<<std::endl;
			std::cout<<"Current Heap Usage: "<<_CurrentByte<<"\t";
			std::cout<<"Maximum Heap Used: "<<_MaxByte<<"\t";
			std::cout<<"Current Temp Mat Usage: "<<_CurrentTempNum<<"\t";
			std::cout<<"Maximum Temp Mat Used: "<<_MaxTempNum<<"\t";
			std::cout<<std::endl;
			std::cout<<"End of Heap Memory Usage Report"<<std::endl<<std::endl;
			return;
		}
#endif
	};
	extern _Mat_Heap MiniMat_HeapScope;
#endif

#define MACON(NAME,TAIL) _##NAME##_##TAIL

//Define a fixed matrix
#define DefMat(TYPE,NAME,ROW,COL) \
	TYPE MACON(NAME,DEFMAT)[((u16) ROW)*((u16) COL)];\
	Mat<TYPE> NAME( (u8) ROW , (u8) COL , ((TYPE *) &(MACON(NAME,DEFMAT)[0])) );

#define RegMat(TYPE,NAME,ROW,COL) \
	extern TYPE MACON(NAME,DEFMAT)[((u16) ROW)*((u16) COL)];\
	extern Mat<TYPE> NAME;

//Define a matrix and copy from a source (destroy the source if temporary)
#define MirMat(TYPE,NAME,ROW,COL,ORIMAT) \
	TYPE MACON(NAME,DEFMAT)[((u16) ROW)*((u16) COL)];\
	Mat<TYPE> NAME( (u8) ROW , (u8) COL , ((TYPE *) &(MACON(NAME,DEFMAT)[0])) );\
	NAME=ORIMAT;

//Define a return matrix (temporary)
#if __HEAPSCOPE
	#define RetMat(TYPE,NAME,ROW,COL) \
			TYPE *MACON(NAME,RETMAT)=(TYPE *) malloc(sizeof(TYPE)*((u16) ROW)*((u16) COL));\
			MiniMat_HeapScope._HeapPlus((sizeof(TYPE)*((u32) ROW)*((u32) COL)));\
			Mat<TYPE> NAME( (u8) ROW , (u8) COL , MACON(NAME,RETMAT) );\
			NAME.SetReturn();
#else
	#define RetMat(TYPE,NAME,ROW,COL) \
			TYPE *MACON(NAME,RETMAT)=(TYPE *) malloc(sizeof(TYPE)*((u16) ROW)*((u16) COL));\
			Mat<TYPE> NAME( (u8) ROW , (u8) COL , MACON(NAME,RETMAT) );\
			NAME.SetReturn();
#endif


template <class Type>

class Mat
{

protected://Variables (None of the variables can be directly accessed)

	Type *_IPtr;//Input Operating Pointer

	bool _IsVector;//Indicate Vector Stat
	bool _IsSquare;//Indicate Square Stat
	bool _IsSingle;//Indicate Single Value Stat

	bool _IsSigned;//Indicate Signed Integer Status;
	bool _IsBool;//Indicate Bool Status
	bool _IsFloat;//Indicate Float Status

	bool _IsReturn;//Indicate Global Variable Status

	Type *_Value;//Root Pointer
	u8 _Row;//Number of Rows
	u8 _Col;//Number of Columns
	u16 _Size;//Number of Elements



public://Functions

	//////Constrcutors & Destructors

	//Global Variable Constructor
	explicit Mat(u8 Row,u8 Col, Type *Value):
	_IPtr(Value),
	_IsVector(0),_IsSquare(0),_IsSingle(0),
	_IsSigned(0),_IsBool(0),_IsFloat(0),
	_IsReturn(0),
	_Value(Value),_Row(Row),_Col(Col)
	{
		if((typeid(Type)==typeid(float))||(typeid(Type)==typeid(double)))
		{
			_IsFloat=1;
		}
		else if(typeid(Type)==typeid(bool))
		{
			_IsBool=1;
		}
		else if((typeid(Type)==typeid(s8))||(typeid(Type)==typeid(s16))||(typeid(Type)==typeid(s32)))
		{
			_IsSigned=1;
		}

		if((_Row==1)&&(_Col==1))
		{
			_IsSingle=1;
		}
		else if ((_Row==1)||(_Col==1))
		{
			_IsVector=1;
		}
		else if (_Row==_Col)
		{
			_IsSquare=1;
		}

		_Size=((u16) _Row)*((u16) _Col);
	};

	//Variable Destructor (Designed for Regional Variable)
	~Mat(){};

	//////End of Constrcutors & Destructors



	//////Content Operation (These Functions Work on Itself)

	//Set Return Status
	Mat<Type> & SetReturn()
	{
		_IsReturn=1;
		return *this;
	}

	//Clean Return AfterUse
	void Clean()
	{
		if(_IsReturn)
		{
#if __HEAPSCOPE
			MiniMat_HeapScope._HeapMinus(((u32)sizeof(Type))*((u32)_Size));
#endif
			free(_Value);
			_Value=NULL;
		}
		return;
	}

	//Insert Single Value
	Mat<Type> & operator()(u8 Row, u8 Col, Type Value)
	{
		Type *_Ptr1=(_Value+(Row-1)*_Col+(Col-1));
		*_Ptr1=Value;
		return *this;
	};

	//Insert Partial Matrix
	Mat<Type> & Insert(u8 RowLow, u8 RowHigh, u8 ColLow, u8 ColHigh, Mat<Type> InsertMat)
	{
		Type *_Ptr1;

		u8 ii,jj;
		for(ii=(RowLow-1);ii<RowHigh;ii++)
		{
			for(jj=(ColLow-1);jj<ColHigh;jj++)
			{
				_Ptr1=_Value+ii*_Col+jj;
				*_Ptr1=*(InsertMat._Value+(ii-RowLow+1)*InsertMat._Col+jj-ColLow+1);
			}
		}

		return *this;
	};

	//Insert Partial Matrix(This will destroy Inserted Mat if is Temp Mat)
	Mat<Type> & operator()(u8 RowLow, u8 RowHigh, u8 ColLow, u8 ColHigh, Mat<Type> InsertMat)
	{
		Type *_Ptr1;

		u8 ii,jj;
		for(ii=(RowLow-1);ii<RowHigh;ii++)
		{
			for(jj=(ColLow-1);jj<ColHigh;jj++)
			{
				_Ptr1=_Value+ii*_Col+jj;
				*_Ptr1=*(InsertMat._Value+(ii-RowLow+1)*InsertMat._Col+jj-ColLow+1);
			}
		}

		InsertMat.Clean();

		return *this;
	};

	//Matrix Equality
	Mat<Type> & Equal(Mat<Type> EqualMat)
	{
		Type *_Ptr1,*_Ptr2;
		_Ptr1=EqualMat._Value;
		_Ptr2=_Value;
		u16 ii;
		for(ii=0;ii<EqualMat._Size;ii++)
		{
			*_Ptr2=*_Ptr1;
			_Ptr1++;
			_Ptr2++;
		}

		return *this;
	}

	//Matrix Equality(This will destroy Input Mat if is Temp Mat)
	Mat<Type> & operator=(Mat<Type> EqualMat)
	{
		Type *_Ptr1,*_Ptr2;
		_Ptr1=EqualMat._Value;
		_Ptr2=_Value;
		u16 ii;
		for(ii=0;ii<EqualMat._Size;ii++)
		{
			*_Ptr2=*_Ptr1;
			_Ptr1++;
			_Ptr2++;
		}

		EqualMat.Clean();

		return *this;
	}

	//Set All Matrix Elements to Same Value
	Mat<Type> & operator=(Type EqualValue)
	{
		Type *_Ptr1=_Value;
		u16 ii;
		for(ii=0;ii<_Size;ii++)
		{
			*_Ptr1=EqualValue;
			_Ptr1++;
		}
		return *this;
	}

	//SubMatrix Equal to an Array;
	Mat<Type> & operator()(u8 RowLow, u8 RowHigh, u8 ColLow, u8 ColHigh, Type *Value)
	{
		Type *_Ptr1=Value;
		Type *_Ptr2;
		u8 ii,jj;
		for(ii=(RowLow-1);ii<RowHigh;ii++)
		{
			for(jj=(ColLow-1);jj<ColHigh;jj++)
			{
				_Ptr2=_Value+ii*_Col+jj;
				*_Ptr2=*_Ptr1;
				_Ptr1++;
			}
		}
		return *this;
	}

	//Swap Row
	Mat<Type> & SwapRow(u8 Row1, u8 Row2)
	{
		Type Mediate;
		Type *_Ptr1=_Value+(Row1-1)*_Col;
		Type *_Ptr2=_Value+(Row2-1)*_Col;
		u8 ii;
		for(ii=0;ii<_Col;ii++)
		{
			Mediate=*_Ptr1;
			*_Ptr1=*_Ptr2;
			*_Ptr2=Mediate;
			_Ptr1++;
			_Ptr2++;
		}
		_Ptr1=_Value;
		_Ptr2=_Value;
		return *this;
	}

	//Swap Col
	Mat<Type> & SwapCol(u8 Col1, u8 Col2)
	{
		Type Mediate;
		Type *_Ptr1=_Value+Col1-1;
		Type *_Ptr2=_Value+Col2-1;
		u8 ii;
		for(ii=0;ii<_Row;ii++)
		{
			Mediate=*_Ptr1;
			*_Ptr1=*_Ptr2;
			*_Ptr2=Mediate;
			_Ptr1+=_Col;
			_Ptr2+=_Col;
		}
		_Ptr1=_Value;
		_Ptr2=_Value;
		return *this;
	}

	//////End of Content operation



	//////Input/Output Flow (Note: Jumper must be used right after the Mat Variable)

	//Inputer (Input)
	Mat<Type> & operator<<(Type SingleValue)
	{
		*_IPtr=SingleValue;
		return *this;
	}

	//Stepper (Step to the Next Col and input)
	Mat<Type> & operator,(Type SingleValue)
	{
		_IPtr++;
		*_IPtr=SingleValue;
		return *this;
	}

	//Jumper (Plus to Jump Right Certain Col, Minus to Jump to First Element of Certain Row, 0 to First Element)
	Mat<Type> & operator>>(s16 SingleValue)
	{
		if(SingleValue==0)
		{
			_IPtr=_Value;
		}
		else if(SingleValue>0)
		{
			_IPtr=_IPtr+SingleValue;
		}
		else
		{
			_IPtr=_Value+(-SingleValue-1)*_Col;
		}
		return *this;
	}

	//Print the Matrix on Console
#ifdef OS_USE_SEMIHOSTING
	void Print()
	{
		std::cout<<std::endl;
		Type *Val=_Value;
		u8 ii,jj;
		for(ii=1;ii<=_Row;ii++)
		{
			for(jj=1;jj<=_Col;jj++)
			{
				std::cout<<(*Val)<<"\t";
				Val++;
			}
			std::cout<<std::endl;
		}
	}
#endif
	//////End of Input Flow



	//////Self Interaction (These functions, if correctly used, generates new things)

	//FeedBack RowNum
	u8 Ro()
	{
		return _Row;
	}

	//FeedBack RowNum
	u8 Co()
	{
		return _Col;
	}

	//FeedBack Size
	u16 Sz()
	{
		return _Size;
	}

	//FeedBack Memory Space
	u32 Sp()
	{
		return _Size*sizeof(Type);
	}

	//Extract Single Value
	Type Val(u8 Row,u8 Col)
	{
		return *(_Value+(Row-1)*_Col+(Col-1));
	}

	//Extract Single Value
	Type operator()(u8 Row, u8 Col)
	{
		Type Val=*(_Value+(Row-1)*_Col+(Col-1));

		Clean();

		return Val;
	}

	//Extract Partial Matrix
	Mat<Type> Val(u8 RowLow, u8 RowHigh, u8 ColLow, u8 ColHigh)
	{
		u8 Row=RowHigh-RowLow+1;
		u8 Col=ColHigh-ColLow+1;

		RetMat(Type,NewMat,Row,Col);

		u8 ii,jj;
		for(ii=(RowLow-1);ii<RowHigh;ii++)
		{
			for(jj=(ColLow-1);jj<ColHigh;jj++)
			{
				NewMat(ii-RowLow+2,jj-ColLow+2,*(_Value+ii*_Col+jj));
			}
		}

		return NewMat;
	}

	//Extract Partial Matrix
	Mat<Type> operator()(u8 RowLow, u8 RowHigh, u8 ColLow, u8 ColHigh)
	{
		u8 Row=RowHigh-RowLow+1;
		u8 Col=ColHigh-ColLow+1;

		RetMat(Type,NewMat,Row,Col);

		u8 ii,jj;
		for(ii=(RowLow-1);ii<RowHigh;ii++)
		{
			for(jj=(ColLow-1);jj<ColHigh;jj++)
			{
				NewMat(ii-RowLow+2,jj-ColLow+2,*(_Value+ii*_Col+jj));
			}
		}

		Clean();

		return NewMat;
	};

	//Calculate Maximum of All Elements
	Type Max()
	{
		Type MaxVal=*_Value;
		Type MaxContender;
		u16 ii;
		for(ii=1;ii<_Size;ii++)
		{
			MaxContender=(*(_Value+ii));
			MaxVal=(MaxVal>MaxContender)?MaxVal:MaxContender;
		}

		return MaxVal;
	}

	//Calculate Maximum of All Element
	friend Type Max(Mat<Type> CalcMat)
	{
		Type MaxVal=*CalcMat._Value;
		Type MaxContender;
		u16 ii;
		for(ii=1;ii<CalcMat._Size;ii++)
		{
			MaxContender=(*(CalcMat._Value+ii));
			MaxVal=(MaxVal>MaxContender)?MaxVal:MaxContender;
		}

		CalcMat.Clean();

		return MaxVal;
	}

	//Calculate Minimum of All Elements
	Type Min()
	{
		Type MinVal=*_Value;
		Type MinContender;

		u16 ii;
		for(ii=1;ii<_Size;ii++)
		{
			MinContender=(*(_Value+ii));
			MinVal=(MinVal<MinContender)?MinVal:MinContender;
		}

		return MinVal;
	}

	//Calculate Minimum of All Element
	friend Type Min(Mat<Type> CalcMat)
	{
		Type MinVal=*CalcMat._Value;
		Type MinContender;
		u16 ii;
		for(ii=1;ii<CalcMat._Size;ii++)
		{
			MinContender=(*(CalcMat._Value+ii));
			MinVal=(MinVal<MinContender)?MinVal:MinContender;
		}

		CalcMat.Clean();

		return MinVal;
	}

	//Calculate Sum of All Element (Return 0 if is boolean)
	Type Sum()
	{
		if(_IsBool)
		{
			return 0;
		}
		else
		{
			Type Sum=0;
			u16 ii;
			for(ii=0;ii<_Size;ii++)
			{
				Sum+=*(_Value+ii);
			}

			return Sum;
		}
	}

	//Calculate Sum of All Element (Return 0 if is boolean)
	friend Type Sum(Mat<Type> CalcMat)
	{
		if(CalcMat._IsBool)
		{
			CalcMat.Clean();

			return 0;
		}
		else
		{
			Type Sum=0;
			u16 ii;
			for(ii=0;ii<CalcMat._Size;ii++)
			{
				Sum+=*(CalcMat._Value+ii);
			}

			CalcMat.Clean();

			return Sum;
		}
	}

	//Calculate L2 Norm of the whole Vector or sqrt(sum(x^2)) of the whole Matrix (Return 0 if is not float)
	Type Norm()
	{
		if(!_IsFloat)
		{
			return 0;
		}
		else
		{
			Type SquareSum=0;
			u16 ii;
			for(ii=0;ii<_Size;ii++)
			{
				SquareSum+=powf(*(_Value+ii),2);
			}

			return sqrtf(SquareSum);
		}
	}

	//Calculate L2 Norm of the whole Vector or sqrt(sum(x^2)) of the whole Matrix (Return 0 if is not float)
	friend Type Norm(Mat<Type> CalcMat)
	{
		if(!CalcMat._IsFloat)
		{
			CalcMat.Clean();

			return 0;
		}
		else
		{
			Type SquareSum=0;
			u16 ii;
			for(ii=0;ii<CalcMat._Size;ii++)
			{
				SquareSum+=powf(*(CalcMat._Value+ii),2);
			}

			CalcMat.Clean();

			return sqrtf(SquareSum);
		}
	}

	//Calculate Absolution of All Elements
	Mat<Type> Abs()
	{
		RetMat(Type,NewMat,_Row,_Col);
		NewMat.Equal(*this);

		Type *Val=NewMat._Value;
		u16 ii;
		for(ii=0;ii<_Size;ii++)
		{
			if(*Val<0)
			{
				*Val=-*Val;
			}
			Val++;
		}
		return NewMat;
	}

	//Calculate Absolution of All Elements (This will destory Operated Matrix if it is Temp Mat)
	friend Mat<Type> Abs(Mat<Type> CalcMat)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		NewMat=CalcMat;

		Type *Val=NewMat._Value;
		u16 ii;
		for(ii=0;ii<CalcMat._Size;ii++)
		{
			if(*Val<0)
			{
				*Val=-*Val;
			}
			Val++;
		}
		return NewMat;
	}

	//*Get SubMatrix of an Element (Return back to itself unchanged if not satisfying type requirement)
	Mat<Type> Sub (u8 Row, u8 Col)
	{
		if ((_Row>=2)&&(_Col>=2))
		{

			if((Row>_Row)||(Row==0)||(Col>_Col)||(Col==0))
			{
				return *this;
			}
			else
			{
				DefMat(Type,OriMat,_Row,_Col);
				OriMat.Equal(*this);
				RetMat(Type,NewMat,_Row-1,_Col-1);

				if((Row==1)&&(Col==1))
				{
					NewMat(1,_Row-1,1,_Col-1,OriMat(2,_Row,2,_Col));
				}
				else if((Row==_Row)&&(Col==_Col))
				{
					NewMat(1,_Row-1,1,_Col-1,OriMat(1,_Row-1,1,_Col-1));
				}
				else if((Row==1)&&(Col==_Col))
				{
					NewMat(1,_Row-1,1,_Col-1,OriMat(2,_Row,1,_Col-1));
				}
				else if((Row==_Row)&&(Col==1))
				{
					NewMat(1,_Row-1,1,_Col-1,OriMat(1,_Row-1,2,_Col));
				}
				else
				{
					if(Row==1)
					{
						NewMat(1,_Row-1,1,Col-1,OriMat(2,_Row,1,Col-1))(1,_Row-1,Col,_Col-1,OriMat(2,_Row,Col+1,_Col));
					}
					else if(Row==_Row)
					{
						NewMat(1,_Row-1,1,Col-1,OriMat(1,_Row-1,1,Col-1))(1,_Row-1,Col,_Col-1,OriMat(1,_Row-1,Col+1,_Col));
					}
					else if(Col==1)
					{
						NewMat(1,Row-1,1,_Col-1,OriMat(1,Row-1,2,_Col))(Row,_Row-1,1,_Col-1,OriMat(Row+1,_Row,2,_Col));
					}
					else if(Col==_Col)
					{
						NewMat(1,Row-1,1,_Col-1,OriMat(1,Row-1,1,_Col-1))(Row,_Row-1,1,_Col-1,OriMat(Row+1,_Row,1,_Col-1));
					}
					else
					{
						NewMat(1,Row-1,1,Col-1,OriMat(1,Row-1,1,Col-1))(Row,_Row-1,Col,_Col-1,OriMat(Row+1,_Row,Col+1,_Col))
							  (1,Row-1,Col,_Col-1,OriMat(1,Row-1,Col+1,_Col))(Row,_Row-1,1,Col-1,OriMat(Row+1,_Row,1,Col-1));
					}
				}

				return NewMat;
			}
		}
		else
		{
			return *this;
		}
	}

	//*Get SubMatrix of an Element (Return back to itself unchanged if not satisfying type requirement)
	friend Mat<Type> Sub(Mat<Type> CalcMat,u8 Row, u8 Col)
	{
		if ((CalcMat._Row>=2)&&(CalcMat._Col>=2))
		{

			if((Row>CalcMat._Row)||(Row==0)||(Col>CalcMat._Col)||(Col==0))
			{
				RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
				NewMat=CalcMat;
				return NewMat;
			}
			else
			{
				MirMat(Type,OriMat,CalcMat._Row,CalcMat._Col,CalcMat);
				RetMat(Type,NewMat,CalcMat._Row-1,CalcMat._Col-1);

				if((Row==1)&&(Col==1))
				{
					NewMat(1,CalcMat._Row-1,1,CalcMat._Col-1,OriMat(2,CalcMat._Row,2,CalcMat._Col));
				}
				else if((Row==CalcMat._Row)&&(Col==CalcMat._Col))
				{
					NewMat(1,CalcMat._Row-1,1,CalcMat._Col-1,OriMat(1,CalcMat._Row-1,1,CalcMat._Col-1));
				}
				else if((Row==1)&&(Col==CalcMat._Col))
				{
					NewMat(1,CalcMat._Row-1,1,CalcMat._Col-1,OriMat(2,CalcMat._Row,1,CalcMat._Col-1));
				}
				else if((Row==CalcMat._Row)&&(Col==1))
				{
					NewMat(1,CalcMat._Row-1,1,CalcMat._Col-1,OriMat(1,CalcMat._Row-1,2,CalcMat._Col));
				}
				else
				{
					if(Row==1)
					{
						NewMat(1,CalcMat._Row-1,1,Col-1,OriMat(2,CalcMat._Row,1,Col-1))(1,CalcMat._Row-1,Col,CalcMat._Col-1,OriMat(2,CalcMat._Row,Col+1,CalcMat._Col));
					}
					else if(Row==CalcMat._Row)
					{
						NewMat(1,CalcMat._Row-1,1,Col-1,OriMat(1,CalcMat._Row-1,1,Col-1))(1,CalcMat._Row-1,Col,CalcMat._Col-1,OriMat(1,CalcMat._Row-1,Col+1,CalcMat._Col));
					}
					else if(Col==1)
					{
						NewMat(1,Row-1,1,CalcMat._Col-1,OriMat(1,Row-1,2,CalcMat._Col))(Row,CalcMat._Row-1,1,CalcMat._Col-1,OriMat(Row+1,CalcMat._Row,2,CalcMat._Col));
					}
					else if(Col==CalcMat._Col)
					{
						NewMat(1,Row-1,1,CalcMat._Col-1,OriMat(1,Row-1,1,CalcMat._Col-1))(Row,CalcMat._Row-1,1,CalcMat._Col-1,OriMat(Row+1,CalcMat._Row,1,CalcMat._Col-1));
					}
					else
					{
						NewMat(1,Row-1,1,Col-1,OriMat(1,Row-1,1,Col-1))(Row,CalcMat._Row-1,Col,CalcMat._Col-1,OriMat(Row+1,CalcMat._Row,Col+1,CalcMat._Col))
							  (1,Row-1,Col,CalcMat._Col-1,OriMat(1,Row-1,Col+1,CalcMat._Col))(Row,CalcMat._Row-1,1,Col-1,OriMat(Row+1,CalcMat._Row,1,Col-1));
					}
				}

				return NewMat;
			}
		}
		else
		{
			RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
			NewMat=CalcMat;
			return NewMat;
		}
	}

	//Calculate Determinant (Return 0 if not satisfying type requirement)
	Type Det()
	{
		if((!_IsSquare)||(_IsBool))
		{
			return 0;
		}
		else
		{
			Type SumVal=0;
			if(_Row==1)
			{
				SumVal=(*_Value);
			}
			else if(_Row==2)
			{
				SumVal=(*_Value)*(*(_Value+3))-(*(_Value+1))*(*(_Value+2));
			}
			else if(_Row==3)
			{
				SumVal=(*_Value)*(*(_Value+4))*(*(_Value+8));
				SumVal+=(*(_Value+1))*(*(_Value+5))*(*(_Value+6));
				SumVal+=(*(_Value+2))*(*(_Value+3))*(*(_Value+7));
				SumVal-=(*_Value)*(*(_Value+7))*(*(_Value+5));
				SumVal-=(*(_Value+1))*(*(_Value+3))*(*(_Value+8));
				SumVal-=(*(_Value+2))*(*(_Value+6))*(*(_Value+4));
			}
			else
			{
				DefMat(Type,OriMat,_Row,_Row);
				OriMat.Equal(*this);
				DefMat(Type,NewMat,(_Row-1),(_Row-1));

				u8 ii;
				Type SignVal,MiniVal;
				for (ii=1;ii<=_Row;ii++)
				{
					if((ii%2)==0)
					{
						SignVal=-1;
					}
					else
					{
						SignVal=1;
					}

					NewMat=OriMat.Sub(1,ii);
					MiniVal=NewMat.Det();
					SumVal=SumVal+SignVal*OriMat(1,ii)*MiniVal;
				}
			}

			return SumVal;
		}
	}

	//Calculate Determinant (Return 0 if not satisfying type requirement)
	friend Type Det(Mat<Type> CalcMat)
	{
		if((!CalcMat._IsSquare)||(CalcMat._IsBool))
		{
			CalcMat.Clean();
			return 0;
		}
		else
		{
			Type SumVal=0;
			Type *Value=CalcMat._Value;
			if(CalcMat._Row==1)
			{
				SumVal=(*Value);
				CalcMat.Clean();
			}
			else if(CalcMat._Row==2)
			{
				SumVal=(*Value)*(*(Value+3))-(*(Value+1))*(*(Value+2));
				CalcMat.Clean();
			}
			else if(CalcMat._Row==3)
			{
				SumVal=(*Value)*(*(Value+4))*(*(Value+8));
				SumVal+=(*(Value+1))*(*(Value+5))*(*(Value+6));
				SumVal+=(*(Value+2))*(*(Value+3))*(*(Value+7));
				SumVal-=(*Value)*(*(Value+7))*(*(Value+5));
				SumVal-=(*(Value+1))*(*(Value+3))*(*(Value+8));
				SumVal-=(*(Value+2))*(*(Value+6))*(*(Value+4));
				CalcMat.Clean();
			}
			else
			{
				MirMat(Type,OriMat,CalcMat._Row,CalcMat._Row,CalcMat);
				DefMat(Type,NewMat,(CalcMat._Row-1),(CalcMat._Row-1));

				u8 ii;
				Type SignVal,MiniVal;
				for (ii=1;ii<=CalcMat._Row;ii++)
				{
					if((ii%2)==0)
					{
						SignVal=-1;
					}
					else
					{
						SignVal=1;
					}

					NewMat=OriMat.Sub(1,ii);
					MiniVal=NewMat.Det();
					SumVal=SumVal+SignVal*OriMat(1,ii)*MiniVal;
				}
			}

			return SumVal;
		}
	}

	//Calculate Transpose
	Mat<Type> Trans()
	{
		RetMat(Type,NewMat,_Col,_Row);

		Type *_Ptr1=_Value;
		u8 ii,jj;
		for (jj=0;jj<_Row;jj++)
		{
			for (ii=0;ii<_Col;ii++)
			{
				NewMat(ii+1,jj+1,*_Ptr1);
				_Ptr1++;
			}
		}
		_Ptr1=_Value;

		return NewMat;
	}

	//Calculate Transpose (This will destory Operated Matrix if it is Temp Mat)
	Mat<Type> operator~()
	{
		RetMat(Type,NewMat,_Col,_Row);

		Type *_Ptr1=_Value;
		u8 ii,jj;
		for (jj=0;jj<_Row;jj++)
		{
			for (ii=0;ii<_Col;ii++)
			{
				NewMat(ii+1,jj+1,*_Ptr1);
				_Ptr1++;
			}
		}
		_Ptr1=_Value;

		Clean();

		return NewMat;
	}

	//Calculate Inverse (Inverse Matrix for Float Types, Reverse for Bool Types, Return back to Itself unchanged for others)
	Mat<Type> Inv()
	{
		if(_IsFloat&&_IsSingle)
		{
			RetMat(Type,InvMat,_Row,_Col);
			InvMat(1,1,1/(*_Value));
			return InvMat;
		}
		if(_IsFloat&&_IsSquare)
		{
			DefMat(Type,OriMat,_Row,_Col);
			OriMat.Equal(*this);
			RetMat(Type,InvMat,_Row,_Col);

			Type TotalDet;
			TotalDet=OriMat.Det();

			if(fabs(TotalDet)>1e-5)
			{
				u8 ii,jj;
				DefMat(Type,MiniMat,_Row-1,_Col-1);
				Type ValSign,MiniDet;
				for (ii=1;ii<=_Row;ii++)
				{
					for (jj=1;jj<=_Col;jj++)
					{
						if(((ii+jj)%2)>0)
						{
							ValSign=-1;
						}
						else
						{
							ValSign=1;
						}

						MiniMat=OriMat.Sub(ii,jj);
						MiniDet=MiniMat.Det();
						InvMat(jj,ii,(ValSign*MiniDet)/TotalDet);
					}
				}
			}
			return InvMat;
		}
		else if(_IsBool)
		{
			RetMat(Type,NewMat,_Row,_Col);
			NewMat.Equal(*this);
			Type *Val=NewMat._Value;
			u16 ii;
			for(ii=0;ii<_Size;ii++)
			{
				*Val=(!(*Val));
				Val++;
			}
			return NewMat;
		}
		else
		{
			return *this;
		}
	}

	//*Calculate Inverse (Inverse Matrix for Float Types, Reverse for Bool Types, Return back to Itself unchanged for others)
	Mat<Type> operator!()
	{
		if(_IsFloat&&_IsSingle)
		{
			RetMat(Type,InvMat,_Row,_Col);
			InvMat(1,1,1/(*_Value));
			Clean();
			return InvMat;
		}
		if(_IsFloat&&_IsSquare)
		{
			MirMat(Type,OriMat,_Row,_Col,*this);
			RetMat(Type,InvMat,_Row,_Col);

			Type TotalDet;
			TotalDet=OriMat.Det();

			if(fabs(TotalDet)>1e-5)
			{
				u8 ii,jj;
				DefMat(Type,MiniMat,_Row-1,_Col-1);
				Type ValSign,MiniDet;
				for (ii=1;ii<=_Row;ii++)
				{
					for (jj=1;jj<=_Col;jj++)
					{
						if(((ii+jj)%2)>0)
						{
							ValSign=-1;
						}
						else
						{
							ValSign=1;
						}

						MiniMat=OriMat.Sub(ii,jj);
						MiniDet=MiniMat.Det();
						InvMat(jj,ii,(ValSign*MiniDet)/TotalDet);
					}
				}
			}
			return InvMat;
		}
		else if(_IsBool)
		{
			RetMat(Type,NewMat,_Row,_Col);
			NewMat=*this;
			Type *Val=NewMat._Value;
			u16 ii;
			for(ii=0;ii<_Size;ii++)
			{
				*Val=(!(*Val));
				Val++;
			}
			return NewMat;
		}
		else
		{
			RetMat(Type,NewMat,_Row,_Col);
			NewMat=*this;
			return NewMat;
		}
	}

	//Plus Sign
	Mat<Type> operator+()
	{
		RetMat(Type,NewMat,_Row,_Col);
		NewMat=*this;
		return NewMat;
	}

	//Minus Sign (Inverted Sign of Signed and Float Types, Return Itself for others)
	Mat<Type> operator-()
	{
		RetMat(Type,NewMat,_Row,_Col);
		NewMat=*this;
		if (_IsSigned||_IsFloat)
		{
			Type *Val=NewMat._Value;
			u16 ii;
			for(ii=0;ii<_Size;ii++)
			{
				*Val=(-(*Val));
				Val++;
			}
		}
		return NewMat;
	}

	//////End of Self Interaction



	//////Interaction Between Matrixes

	//Calculate Plus
	friend Mat<Type> operator+(Mat<Type> CalcMatL, Mat<Type> CalcMatR)
	{
		RetMat(Type,NewMat,CalcMatL._Row,CalcMatR._Col);
		Type *LPtr=CalcMatL._Value;
		Type *RPtr=CalcMatR._Value;
		u8 ii,jj;
		if(!NewMat._IsBool)
		{
			for (ii=1;ii<=NewMat._Row;ii++)
			{
				for (jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*LPtr+*RPtr));
					LPtr++;
					RPtr++;
				}
			}
		}

		CalcMatL.Clean();
		CalcMatR.Clean();

		return NewMat;
	}

	//Calculate Minus
	friend Mat<Type> operator-(Mat<Type> CalcMatL, Mat<Type> CalcMatR)
	{
		RetMat(Type,NewMat,CalcMatL._Row,CalcMatR._Col);
		Type *LPtr=CalcMatL._Value;
		Type *RPtr=CalcMatR._Value;
		u8 ii,jj;
		if(!NewMat._IsBool)
		{
			for (ii=1;ii<=NewMat._Row;ii++)
			{
				for (jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*LPtr-*RPtr));
					LPtr++;
					RPtr++;
				}
			}
		}

		CalcMatL.Clean();
		CalcMatR.Clean();

		return NewMat;
	}

	//Calculate Multiply
	friend Mat<Type> operator*(Mat<Type> CalcMatL, Mat<Type> CalcMatR)
	{
		RetMat(Type,NewMat,CalcMatL._Row,CalcMatR._Col);
		Type *LPtr,*RPtr;
		Type MultSum=0;
		u8 ii,jj,kk;
		if(!NewMat._IsBool)
		{
			for (ii=1;ii<=CalcMatL._Row;ii++)
			{
				for (jj=1;jj<=CalcMatR._Col;jj++)
				{
					LPtr=CalcMatL._Value+(ii-1)*CalcMatR._Col;
					RPtr=CalcMatR._Value+jj-1;
					for (kk=1;kk<=CalcMatL._Col;kk++)
					{
						MultSum+=(*LPtr)*(*RPtr);
						LPtr++;
						RPtr+=CalcMatR._Col;
					}
					NewMat(ii,jj,MultSum);
					MultSum=0;
				}
			}
		}

		CalcMatL.Clean();
		CalcMatR.Clean();

		return NewMat;
	}

	//Calculate Equal from Plus
	Mat<Type> & operator+=(Mat<Type> CalcMat)
	{
		Type *LPtr=_Value;
		Type *RPtr=CalcMat._Value;
		u8 ii,jj;
		if(!_IsBool)
		{
			for (ii=1;ii<=_Row;ii++)
			{
				for (jj=1;jj<=_Col;jj++)
				{
					this->operator ()(ii,jj,(*LPtr+*RPtr));
					LPtr++;
					RPtr++;
				}
			}
		}

		CalcMat.Clean();

		return *this;
	}

	//Calculate Equal from Minus
	Mat<Type> & operator-=(Mat<Type> CalcMat)
	{
		Type *LPtr=_Value;
		Type *RPtr=CalcMat._Value;
		u8 ii,jj;
		if(!_IsBool)
		{
			for (ii=1;ii<=_Row;ii++)
			{
				for (jj=1;jj<=_Col;jj++)
				{
					this->operator ()(ii,jj,(*LPtr-*RPtr));
					LPtr++;
					RPtr++;
				}
			}
		}

		CalcMat.Clean();

		return *this;
	}

	//Calculate Equal from Multiply
	Mat<Type> & operator*=(Mat<Type> CalcMat)
	{
		Type *LPtr,*RPtr;
		Type MultSum=0;
		u8 ii,jj,kk;
		if(!_IsBool)
		{
			for (ii=1;ii<=_Row;ii++)
			{
				for (jj=1;jj<=CalcMat._Col;jj++)
				{
					LPtr=_Value+(ii-1)*_Col;
					RPtr=CalcMat._Value+jj-1;
					for (kk=1;kk<=_Col;kk++)
					{
						MultSum+=(*LPtr)*(*RPtr);
						LPtr++;
						RPtr+=CalcMat._Col;
					}
					this->operator ()(ii,jj,MultSum);
					MultSum=0;
				}
			}
		}

		CalcMat.Clean();

		return *this;
	}

	//Calculate Point Product
	friend Type Point(Mat<Type> CalcMatL, Mat<Type> CalcMatR)
	{
		Type SumVal=0;
		Type *LPtr=CalcMatL._Value;
		Type *RPtr=CalcMatR._Value;
		u8 ii,jj;
		if((!CalcMatL._IsBool)&&(!CalcMatR._IsBool))
		{
			for (ii=1;ii<=CalcMatL._Row;ii++)
			{
				for (jj=1;jj<=CalcMatR._Col;jj++)
				{
					SumVal+=((*LPtr)*(*RPtr));
					LPtr++;
					RPtr++;
				}
			}
		}

		CalcMatL.Clean();
		CalcMatR.Clean();

		return SumVal;
	}

	//Calculate Cross Product
	friend Mat<Type> Cross(Mat<Type> CalcMatL, Mat<Type> CalcMatR)
	{
		RetMat(Type,NewMat,3,1);
		if(CalcMatL._IsVector&&CalcMatR._IsVector&&(CalcMatL._Size==3)&&(CalcMatR._Size=3))
		{
			Type *ValL=CalcMatL._Value;
			Type *ValR=CalcMatR._Value;

			NewMat(1,1,+((*(ValL+1))*(*(ValR+2))-(*(ValR+1))*(*(ValL+2))));
			NewMat(2,1,-((*(ValL))*(*(ValR+2))-(*(ValR))*(*(ValL+2))));
			NewMat(3,1,+((*(ValL))*(*(ValR+1))-(*(ValR))*(*(ValL+1))));
		}
		return NewMat;
	}

	//////End of Interaction Between Matrixes



	//////Interaction with Back Single Values

	//Calculate Plus
	friend Mat<Type> operator+(Mat<Type> CalcMat, Type CalcValue)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;
		if(!NewMat._IsBool)
		{
			for (ii=1;ii<=NewMat._Row;ii++)
			{
				for (jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*LPtr)+CalcValue);
					LPtr++;
				}
			}
		}

		CalcMat.Clean();

		return NewMat;
	}

	//Calculate Minus
	friend Mat<Type> operator-(Mat<Type> CalcMat, Type CalcValue)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;
		if(!NewMat._IsBool)
		{
			for (ii=1;ii<=NewMat._Row;ii++)
			{
				for (jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*LPtr)-CalcValue);
					LPtr++;
				}
			}
		}

		CalcMat.Clean();

		return NewMat;
	}

	//Calculate Multiply
	friend Mat<Type> operator*(Mat<Type> CalcMat, Type CalcValue)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;
		if(!NewMat._IsBool)
		{
			for (ii=1;ii<=NewMat._Row;ii++)
			{
				for (jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*LPtr)*CalcValue);
					LPtr++;
				}
			}
		}

		CalcMat.Clean();

		return NewMat;
	}

	//Calculate Devide
	friend Mat<Type> operator/(Mat<Type> CalcMat, Type CalcValue)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;
		if(!NewMat._IsBool)
		{
			for (ii=1;ii<=NewMat._Row;ii++)
			{
				for (jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*LPtr)/CalcValue);
					LPtr++;
				}
			}
		}

		CalcMat.Clean();

		return NewMat;
	}

	//Calculate Equal from Plus
	Mat<Type> & operator+=(Type CalcValue)
	{
		Type *LPtr=_Value;
		u8 ii,jj;
		if(!_IsBool)
		{
			for (ii=1;ii<=_Row;ii++)
			{
				for (jj=1;jj<=_Col;jj++)
				{
					this->operator ()(ii,jj,(*LPtr)+CalcValue);
					LPtr++;
				}
			}
		}

		return *this;
	}

	//Calculate Equal from Minus
	Mat<Type> & operator-=(Type CalcValue)
	{
		Type *LPtr=_Value;
		u8 ii,jj;
		if(!_IsBool)
		{
			for (ii=1;ii<=_Row;ii++)
			{
				for (jj=1;jj<=_Col;jj++)
				{
					this->operator ()(ii,jj,(*LPtr)-CalcValue);
					LPtr++;
				}
			}
		}

		return *this;
	}

	//Calculate Equal from Multiply
	Mat<Type> & operator*=(Type CalcValue)
	{
		Type *LPtr=_Value;
		u8 ii,jj;
		if(!_IsBool)
		{
			for (ii=1;ii<=_Row;ii++)
			{
				for (jj=1;jj<=_Col;jj++)
				{
					this->operator ()(ii,jj,(*LPtr)*CalcValue);
					LPtr++;
				}
			}
		}

		return *this;
	}

	//Calculate Equal from Devide
	Mat<Type> & operator/=(Type CalcValue)
	{
		Type *LPtr=_Value;
		u8 ii,jj;
		if(!_IsBool)
		{
			for (ii=1;ii<=_Row;ii++)
			{
				for (jj=1;jj<=_Col;jj++)
				{
					this->operator ()(ii,jj,(*LPtr)/CalcValue);
					LPtr++;
				}
			}
		}

		return *this;
	}

	//Calculate Plus
	friend Mat<Type> operator+(Type CalcValue, Mat<Type> CalcMat)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;
		if(!NewMat._IsBool)
		{
			for (ii=1;ii<=NewMat._Row;ii++)
			{
				for (jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*LPtr)+CalcValue);
					LPtr++;
				}
			}
		}

		CalcMat.Clean();

		return NewMat;
	}

	//Calculate Minus
	friend Mat<Type> operator-(Type CalcValue, Mat<Type> CalcMat)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;
		if(!NewMat._IsBool)
		{
			for (ii=1;ii<=NewMat._Row;ii++)
			{
				for (jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*LPtr)-CalcValue);
					LPtr++;
				}
			}
		}

		CalcMat.Clean();

		return NewMat;
	}

	//Calculate Multiply
	friend Mat<Type> operator*(Type CalcValue, Mat<Type> CalcMat)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;
		if(!NewMat._IsBool)
		{
			for (ii=1;ii<=NewMat._Row;ii++)
			{
				for (jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*LPtr)*CalcValue);
					LPtr++;
				}
			}
		}

		CalcMat.Clean();

		return NewMat;
	}

	//////End of Interaction Bwith Single Values




	//////Comparison and Bool Interactions (Use with Cautious, Since they feedback exact types)

	//Compare Bigger
	friend Mat<bool> operator>(Mat<Type> CompareMatL,Mat<Type> CompareMatR)
	{
		RetMat(bool,NewMat,CompareMatL._Row,CompareMatR._Col);
		Type *LPtr=CompareMatL._Value;
		Type *RPtr=CompareMatR._Value;

		if((!CompareMatL._IsBool)&&(!CompareMatR._IsBool))
		{
			u8 ii,jj;
			for (ii=1;ii<=CompareMatL._Row;ii++)
			{
				for (jj=1;jj<=CompareMatR._Col;jj++)
				{
					NewMat(ii,jj,((*LPtr)>(*RPtr))?1:0);
					LPtr++;
					RPtr++;
				}
			}
		}

		CompareMatL.Clean();
		CompareMatR.Clean();

		return NewMat;
	}

	//Compare Smaller
	friend Mat<bool> operator<(Mat<Type> CompareMatL,Mat<Type> CompareMatR)
	{
		RetMat(bool,NewMat,CompareMatL._Row,CompareMatR._Col);
		Type *LPtr=CompareMatL._Value;
		Type *RPtr=CompareMatR._Value;

		if((!CompareMatL._IsBool)&&(!CompareMatR._IsBool))
		{
			u8 ii,jj;
			for (ii=1;ii<=CompareMatL._Row;ii++)
			{
				for (jj=1;jj<=CompareMatR._Col;jj++)
				{
					NewMat(ii,jj,((*LPtr)<(*RPtr))?1:0);
					LPtr++;
					RPtr++;
				}
			}
		}

		CompareMatL.Clean();
		CompareMatR.Clean();

		return NewMat;
	}

	//Compare Equal
	friend Mat<bool> operator==(Mat<Type> CompareMatL,Mat<Type> CompareMatR)
	{
		RetMat(bool,NewMat,CompareMatL._Row,CompareMatR._Col);
		Type *LPtr=CompareMatL._Value;
		Type *RPtr=CompareMatR._Value;

		if((!CompareMatL._IsBool)&&(!CompareMatR._IsBool))
		{
			u8 ii,jj;
			for (ii=1;ii<=CompareMatL._Row;ii++)
			{
				for (jj=1;jj<=CompareMatR._Col;jj++)
				{
					NewMat(ii,jj,((*LPtr)==(*RPtr))?1:0);
					LPtr++;
					RPtr++;
				}
			}
		}

		CompareMatL.Clean();
		CompareMatR.Clean();

		return NewMat;
	}

	//Compare Inequal
	friend Mat<bool> operator!=(Mat<Type> CompareMatL,Mat<Type> CompareMatR)
	{
		RetMat(bool,NewMat,CompareMatL._Row,CompareMatR._Col);
		Type *LPtr=CompareMatL._Value;
		Type *RPtr=CompareMatR._Value;

		if((!CompareMatL._IsBool)&&(!CompareMatR._IsBool))
		{
			u8 ii,jj;
			for (ii=1;ii<=CompareMatL._Row;ii++)
			{
				for (jj=1;jj<=CompareMatR._Col;jj++)
				{
					NewMat(ii,jj,((*LPtr)!=(*RPtr))?1:0);
					LPtr++;
					RPtr++;
				}
			}
		}

		CompareMatL.Clean();
		CompareMatR.Clean();

		return NewMat;
	}

	//Compare No Smaller
	friend Mat<bool> operator>=(Mat<Type> CompareMatL,Mat<Type> CompareMatR)
	{
		RetMat(bool,NewMat,CompareMatL._Row,CompareMatR._Col);
		Type *LPtr=CompareMatL._Value;
		Type *RPtr=CompareMatR._Value;

		if((!CompareMatL._IsBool)&&(!CompareMatR._IsBool))
		{
			u8 ii,jj;
			for (ii=1;ii<=CompareMatL._Row;ii++)
			{
				for (jj=1;jj<=CompareMatR._Col;jj++)
				{
					NewMat(ii,jj,((*LPtr)>=(*RPtr))?1:0);
					LPtr++;
					RPtr++;
				}
			}
		}

		CompareMatL.Clean();
		CompareMatR.Clean();

		return NewMat;
	}

	//Compare No Bigger
	friend Mat<bool> operator<=(Mat<Type> CompareMatL,Mat<Type> CompareMatR)
	{
		RetMat(bool,NewMat,CompareMatL._Row,CompareMatR._Col);
		Type *LPtr=CompareMatL._Value;
		Type *RPtr=CompareMatR._Value;

		if((!CompareMatL._IsBool)&&(!CompareMatR._IsBool))
		{
			u8 ii,jj;
			for (ii=1;ii<=CompareMatL._Row;ii++)
			{
				for (jj=1;jj<=CompareMatR._Col;jj++)
				{
					NewMat(ii,jj,((*LPtr)<=(*RPtr))?1:0);
					LPtr++;
					RPtr++;
				}
			}
		}

		CompareMatL.Clean();
		CompareMatR.Clean();

		return NewMat;
	}

	//Logic AND
	friend Mat<bool> operator&(Mat<Type> BoolMatL,Mat<Type> BoolMatR)
	{
		RetMat(bool,NewMat,BoolMatL._Row,BoolMatR._Col);
		bool *ValL=BoolMatL._Value;
		bool *ValR=BoolMatR._Value;

		if(BoolMatL._IsBool&&BoolMatR._IsBool)
		{
			u8 ii,jj;
			for(ii=1;ii<=NewMat._Row;ii++)
			{
				for(jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*ValL)&(*ValR));
					ValL++;
					ValR++;
				}
			}
		}

		BoolMatL.Clean();
		BoolMatR.Clean();

		return NewMat;
	}

	//Logic OR
	friend Mat<bool> operator|(Mat<Type> BoolMatL,Mat<Type> BoolMatR)
	{
		RetMat(bool,NewMat,BoolMatL._Row,BoolMatR._Col);
		bool *ValL=BoolMatL._Value;
		bool *ValR=BoolMatR._Value;

		if(BoolMatL._IsBool&&BoolMatR._IsBool)
		{
			u8 ii,jj;
			for(ii=1;ii<=NewMat._Row;ii++)
			{
				for(jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*ValL)|(*ValR));
					ValL++;
					ValR++;
				}
			}
		}

		BoolMatL.Clean();
		BoolMatR.Clean();

		return NewMat;
	}

	//Logic XOR
	friend Mat<bool> operator^(Mat<Type> BoolMatL,Mat<Type> BoolMatR)
	{
		RetMat(bool,NewMat,BoolMatL._Row,BoolMatR._Col);
		bool *ValL=BoolMatL._Value;
		bool *ValR=BoolMatR._Value;

		if(BoolMatL._IsBool&&BoolMatR._IsBool)
		{
			u8 ii,jj;
			for(ii=1;ii<=NewMat._Row;ii++)
			{
				for(jj=1;jj<=NewMat._Col;jj++)
				{
					NewMat(ii,jj,(*ValL)^(*ValR));
					ValL++;
					ValR++;
				}
			}
		}

		BoolMatL.Clean();
		BoolMatR.Clean();

		return NewMat;
	}

	//Equal from Logic AND
	Mat<bool> & operator&=(Mat<bool> BoolMat)
	{
		if(_IsBool)
		{
			bool *ValL=_Value;
			bool *ValR=BoolMat._Value;

			u8 ii,jj;
			for(ii=1;ii<=_Row;ii++)
			{
				for(jj=1;jj<=_Col;jj++)
				{
					this->operator()(ii,jj,(*ValL)&(*ValR));
					ValL++;
					ValR++;
				}
			}

			BoolMat.Clean();
		}

		return *this;
	}

	//Equal from Logic OR
	Mat<bool> & operator|=(Mat<bool> BoolMat)
	{
		if(_IsBool)
		{
			bool *ValL=_Value;
			bool *ValR=BoolMat._Value;

			u8 ii,jj;
			for(ii=1;ii<=_Row;ii++)
			{
				for(jj=1;jj<=_Col;jj++)
				{
					this->operator()(ii,jj,(*ValL)|=(*ValR));
					ValL++;
					ValR++;
				}
			}

			BoolMat.Clean();
		}

		return *this;
	}

	//Equal from Logic XOR
	Mat<bool> & operator^=(Mat<bool> BoolMat)
	{
		if(_IsBool)
		{
			bool *ValL=_Value;
			bool *ValR=BoolMat._Value;

			u8 ii,jj;
			for(ii=1;ii<=_Row;ii++)
			{
				for(jj=1;jj<=_Col;jj++)
				{
					this->operator()(ii,jj,(*ValL)^=(*ValR));
					ValL++;
					ValR++;
				}
			}

			BoolMat.Clean();
		}

		return *this;
	}

	//////End of Comparison and Bool Interactions

};

#endif
#endif

