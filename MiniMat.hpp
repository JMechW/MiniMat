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

#ifndef MINIMAT_HPP_
#define MINIMAT_HPP_

#ifndef _MICROCHIP
	#define _MICROCHIP 0
#endif

#ifndef _HEAPSCOPE
	#define _HEAPSCOPE 1
#endif

#include <stdlib.h>
#include <cmath>

#if !_MICROCHIP
	#include <iostream>
	typedef unsigned char u8;
	typedef unsigned short u16;
	typedef unsigned long u32;
	typedef signed char s8;
	typedef signed short s16;
	typedef signed long s32;
#endif

using namespace std;

#if _HEAPSCOPE
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

		#if !_MICROCHIP
		void Print()
		{
			cout<<endl;
			cout<<"Heap Memory Usage Report:"<<endl;
			cout<<"Current Heap Usage: "<<_CurrentByte<<"\t";
			cout<<"Maximum Heap Used: "<<_MaxByte<<"\t";
			cout<<"Current Temp Mat Usage: "<<_CurrentTempNum<<"\t";
			cout<<"Maximum Temp Mat Used: "<<_MaxTempNum<<"\t";
			cout<<endl;
			cout<<"End of Heap Memory Usage Report"<<endl<<endl;
			return;
		}
		#endif
	};
	extern _Mat_Heap MiniMat_HeapScope;
#endif



#define MACON(NAME,TAIL) _##NAME##_##TAIL

//Define a fixed matrix
#define DefMat(TYPE,NAME,ROW,COL) \
	TYPE MACON(NAME,DEFMAT)[((const u16) ROW)*((const u16) COL)];\
	Mat<TYPE> NAME( (u8) ROW , (u8) COL , ((TYPE *) &(MACON(NAME,DEFMAT)[0])) );

//Define a return matrix (temporary)
#if _HEAPSCOPE
	#define TmpMat(TYPE,NAME,ROW,COL) \
			TYPE *MACON(NAME,TMPMAT)=(TYPE *) malloc(sizeof(TYPE)*((const u16) ROW)*((const u16) COL));\
			MiniMat_HeapScope._HeapPlus((sizeof(TYPE)*((u32) ROW)*((u32) COL)));\
			Mat<TYPE> NAME( (u8) ROW , (u8) COL , MACON(NAME,TMPMAT) );\
			NAME.SetTemp();

	#define RetMat(TYPE,NAME,ROW,COL) \
			TYPE *MACON(NAME,RETMAT)=(TYPE *) malloc(sizeof(TYPE)*((const u16) ROW)*((const u16) COL));\
			MiniMat_HeapScope._HeapPlus((sizeof(TYPE)*((u32) ROW)*((u32) COL)));\
			Mat<TYPE> NAME( (u8) ROW , (u8) COL , MACON(NAME,RETMAT) );\
			NAME.SetReturn();
#else
	#define TmpMat(TYPE,NAME,ROW,COL) \
			TYPE *MACON(NAME,TMPMAT)=(TYPE *) malloc(sizeof(TYPE)*((const u16) ROW)*((const u16) COL));\
			Mat<TYPE> NAME( (u8) ROW , (u8) COL , MACON(NAME,TMPMAT) );\
			NAME.SetTemp();

	#define RetMat(TYPE,NAME,ROW,COL) \
			TYPE *MACON(NAME,RETMAT)=(TYPE *) malloc(sizeof(TYPE)*((const u16) ROW)*((const u16) COL));\
			Mat<TYPE> NAME( (u8) ROW , (u8) COL , MACON(NAME,RETMAT) );\
			NAME.SetReturn();
#endif

//Define a matrix and copy from a source (destroy the source if temporary)
#define MirDefMat(TYPE,NAME,ROW,COL,ORIMAT) \
		DefMat(TYPE,NAME,ROW,COL); \
		NAME=ORIMAT;

#define MirTmpMat(TYPE,NAME,ROW,COL,ORIMAT) \
		TmpMat(TYPE,NAME,ROW,COL);\
		NAME=ORIMAT;

#define MirRetMat(TYPE,NAME,ROW,COL,ORIMAT) \
		RetMat(TYPE,NAME,ROW,COL);\
		NAME=ORIMAT;



template <class Type>

class Mat
{

protected://Variables (None of the variables can be directly accessed)

	Type *_IPtr;//Input Operating Pointer

	bool _IsVector;//Indicate Vector Stat
	bool _IsSquare;//Indicate Square Stat
	bool _IsSingle;//Indicate Single Value Stat

	bool _IsTemp;//Indicate Temperorary Status
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
	_IsTemp(0),_IsReturn(0),
	_Value(Value),_Row(Row),_Col(Col)
	{

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

	//Set Temp Status
	Mat<Type> & SetTemp()
	{
		_IsTemp=1;
		return *this;
	}

	//Set Return Status
	Mat<Type> & SetReturn()
	{
		_IsReturn=1;
		return *this;
	}

	//Clean Temp
	void CleanTmp()
	{
		if(_IsTemp)
		{
#if _HEAPSCOPE
			MiniMat_HeapScope._HeapMinus(((u32)sizeof(Type))*((u32)_Size));
#endif
			free(_Value);
			_Value=NULL;
		}
		return;
	}

	//Clean Return After Use
	void CleanRet()
	{
		if(_IsReturn)
		{
#if _HEAPSCOPE
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

		InsertMat.CleanRet();

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

		EqualMat.CleanRet();

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
#if !_MICROCHIP
	void Print()
	{
		cout<<endl;
		Type *Val=_Value;
		u8 ii,jj;
		for(ii=1;ii<=_Row;ii++)
		{
			for(jj=1;jj<=_Col;jj++)
			{
				cout<<(*Val)<<"\t";
				Val++;
			}
			cout<<endl;
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

		CleanRet();

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

		CleanRet();

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

		CalcMat.CleanRet();

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

		CalcMat.CleanRet();

		return MinVal;
	}

	//Calculate Sum of All Element (Well be useless if is boolean)
	Type Sum()
	{	Type Sum=0;
			u16 ii;
			for(ii=0;ii<_Size;ii++)
			{
				Sum+=*(_Value+ii);
			}

		return Sum;
	}

	//Calculate Sum of All Element (Well be useless if is boolean)
	friend Type Sum(Mat<Type> CalcMat)
	{
		Type Sum=0;
		u16 ii;
		for(ii=0;ii<CalcMat._Size;ii++)
		{
			Sum+=*(CalcMat._Value+ii);
		}

		CalcMat.CleanRet();

		return Sum;
	}

	//Calculate L2 Norm of the whole Vector or sqrt(sum(x^2)) of the whole Matrix (Will be Error if is not float)
	Type Norm()
	{
		Type SquareSum=0;
		u16 ii;
		for(ii=0;ii<_Size;ii++)
		{
			SquareSum+=powf(*(_Value+ii),2);
		}

		return sqrtf(SquareSum);
	}

	//Calculate L2 Norm of the whole Vector or sqrt(sum(x^2)) of the whole Matrix (Will be Error if is not float)
	friend Type Norm(Mat<Type> CalcMat)
	{
		Type SquareSum=0;
		u16 ii;
		for(ii=0;ii<CalcMat._Size;ii++)
		{
			SquareSum+=powf(*(CalcMat._Value+ii),2);
		}

		CalcMat.CleanRet();

		return sqrtf(SquareSum);
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

	//*Get SubMatrix of an Element (Will be wrong if not satisfying type requirement)
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
				TmpMat(Type,OriMat,_Row,_Col);
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

				OriMat.CleanTmp();
				return NewMat;
			}
		}
		else
		{
			return *this;
		}
	}

	//*Get SubMatrix of an Element (Will be wrong if not satisfying type requirement)
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
				MirTmpMat(Type,OriMat,CalcMat._Row,CalcMat._Col,CalcMat);
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

				OriMat.CleanTmp();
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

	//Calculate Determinant (Return 0 if not satisfying size requirement)
	Type Det()
	{
		if((!_IsSquare))
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
				TmpMat(Type,OriMat,_Row,_Row);
				OriMat.Equal(*this);
				TmpMat(Type,NewMat,(_Row-1),(_Row-1));

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

				OriMat.CleanTmp();
				NewMat.CleanTmp();
			}

			return SumVal;
		}
	}

	//Calculate Determinant (Return 0 if not satisfying size requirement)
	friend Type Det(Mat<Type> CalcMat)
	{
		if((!CalcMat._IsSquare))
		{
			CalcMat.CleanRet();
			return 0;
		}
		else
		{
			Type SumVal=0;
			Type *Value=CalcMat._Value;
			if(CalcMat._Row==1)
			{
				SumVal=(*Value);
				CalcMat.CleanRet();
			}
			else if(CalcMat._Row==2)
			{
				SumVal=(*Value)*(*(Value+3))-(*(Value+1))*(*(Value+2));
				CalcMat.CleanRet();
			}
			else if(CalcMat._Row==3)
			{
				SumVal=(*Value)*(*(Value+4))*(*(Value+8));
				SumVal+=(*(Value+1))*(*(Value+5))*(*(Value+6));
				SumVal+=(*(Value+2))*(*(Value+3))*(*(Value+7));
				SumVal-=(*Value)*(*(Value+7))*(*(Value+5));
				SumVal-=(*(Value+1))*(*(Value+3))*(*(Value+8));
				SumVal-=(*(Value+2))*(*(Value+6))*(*(Value+4));
				CalcMat.CleanRet();
			}
			else
			{
				MirTmpMat(Type,OriMat,CalcMat._Row,CalcMat._Row,CalcMat);
				TmpMat(Type,NewMat,(CalcMat._Row-1),(CalcMat._Row-1));

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

				OriMat.CleanTmp();
				NewMat.CleanTmp();
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

		CleanRet();

		return NewMat;
	}

	//Calculate Inverse (Inverse Matrix for Float Types, Reverse for Bool Types, Return back to Itself unchanged for others)
	Mat<Type> Inv()
	{
		if(_IsSingle)
		{
			RetMat(Type,InvMat,_Row,_Col);
			InvMat(1,1,1/(*_Value));
			return InvMat;
		}
		if(_IsSquare)
		{
			TmpMat(Type,OriMat,_Row,_Col);
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
			OriMat.CleanTmp();
			return InvMat;
		}
		else
		{
			return *this;
		}
	}

	//*Calculate Inverse (Inverse Matrix for Float Types, Reverse for Bool Types, Return back to Itself unchanged for others)
	Mat<Type> operator!()
	{
		if(_IsSingle)
		{
			RetMat(Type,InvMat,_Row,_Col);
			InvMat(1,1,1/(*_Value));
			CleanRet();
			return InvMat;
		}
		if(_IsSquare)
		{
			MirTmpMat(Type,OriMat,_Row,_Col,*this);
			RetMat(Type,InvMat,_Row,_Col);

			Type TotalDet;
			TotalDet=OriMat.Det();

			if(fabs(TotalDet)>1e-5)
			{
				u8 ii,jj;
				TmpMat(Type,MiniMat,_Row-1,_Col-1);
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
				MiniMat.CleanTmp();
			}

			OriMat.CleanTmp();
			return InvMat;
		}
		else
		{
			RetMat(Type,NewMat,_Row,_Col);
			NewMat=*this;
			return NewMat;
		}
	}

	//Plus Sign (Basically Useless)
	Mat<Type> operator+()
	{
		RetMat(Type,NewMat,_Row,_Col);
		NewMat=*this;
		return NewMat;
	}

	//Minus Sign (Inverted Sign of Signed and Float Types, Not Applicable for other types)
	Mat<Type> operator-()
	{
		RetMat(Type,NewMat,_Row,_Col);
		NewMat=*this;
		Type *Val=NewMat._Value;
		u16 ii;
		for(ii=0;ii<_Size;ii++)
		{
			*Val=(-(*Val));
			Val++;
		}
		return NewMat;
	}

	//Negative Operation (Only Useful for Boolean, will destroy Operated Matrix if is Temp Mat)
	Mat<bool> Neg()
	{
		MirRetMat(bool,NewMat,_Row,_Col,*this);
		Type *Val=NewMat._Value;
		u16 ii;
		for(ii=0;ii<_Size;ii++)
		{
			*Val=(!(*Val));
			Val++;
		}
		return NewMat;
	}

	//Negative Operation (Only Useful for Boolean, will destroy Operated Matrix if is Temp Mat)
	friend Mat<bool> Neg(Mat<Type> CalcMat)
	{
		MirRetMat(bool,NewMat,CalcMat._Row,CalcMat._Col,CalcMat);
		Type *Val=NewMat._Value;
		u16 ii;
		for(ii=0;ii<NewMat._Size;ii++)
		{
			*Val=(!(*Val));
			Val++;
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

		for (ii=1;ii<=NewMat._Row;ii++)
		{
			for (jj=1;jj<=NewMat._Col;jj++)
			{
				NewMat(ii,jj,(*LPtr+*RPtr));
				LPtr++;
				RPtr++;
			}
		}

		CalcMatL.CleanRet();
		CalcMatR.CleanRet();

		return NewMat;
	}

	//Calculate Minus
	friend Mat<Type> operator-(Mat<Type> CalcMatL, Mat<Type> CalcMatR)
	{
		RetMat(Type,NewMat,CalcMatL._Row,CalcMatR._Col);
		Type *LPtr=CalcMatL._Value;
		Type *RPtr=CalcMatR._Value;
		u8 ii,jj;

		for (ii=1;ii<=NewMat._Row;ii++)
		{
			for (jj=1;jj<=NewMat._Col;jj++)
			{
				NewMat(ii,jj,(*LPtr-*RPtr));
				LPtr++;
				RPtr++;
			}
		}

		CalcMatL.CleanRet();
		CalcMatR.CleanRet();

		return NewMat;
	}

	//Calculate Multiply
	friend Mat<Type> operator*(Mat<Type> CalcMatL, Mat<Type> CalcMatR)
	{
		RetMat(Type,NewMat,CalcMatL._Row,CalcMatR._Col);
		Type *LPtr,*RPtr;
		Type MultSum=0;
		u8 ii,jj,kk;

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


		CalcMatL.CleanRet();
		CalcMatR.CleanRet();

		return NewMat;
	}

	//Calculate Equal from Plus
	Mat<Type> & operator+=(Mat<Type> CalcMat)
	{
		Type *LPtr=_Value;
		Type *RPtr=CalcMat._Value;
		u8 ii,jj;

		for (ii=1;ii<=_Row;ii++)
		{
			for (jj=1;jj<=_Col;jj++)
			{
				this->operator ()(ii,jj,(*LPtr+*RPtr));
				LPtr++;
				RPtr++;
			}
		}


		CalcMat.CleanRet();

		return *this;
	}

	//Calculate Equal from Minus
	Mat<Type> & operator-=(Mat<Type> CalcMat)
	{
		Type *LPtr=_Value;
		Type *RPtr=CalcMat._Value;
		u8 ii,jj;


		for (ii=1;ii<=_Row;ii++)
		{
			for (jj=1;jj<=_Col;jj++)
			{
				this->operator ()(ii,jj,(*LPtr-*RPtr));
				LPtr++;
				RPtr++;
			}
		}


		CalcMat.CleanRet();

		return *this;
	}

	//Calculate Equal from Multiply
	Mat<Type> & operator*=(Mat<Type> CalcMat)
	{
		Type *LPtr,*RPtr;
		Type MultSum=0;
		u8 ii,jj,kk;

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

		CalcMat.CleanRet();

		return *this;
	}

	//Calculate Point Product
	friend Type Point(Mat<Type> CalcMatL, Mat<Type> CalcMatR)
	{
		Type SumVal=0;
		Type *LPtr=CalcMatL._Value;
		Type *RPtr=CalcMatR._Value;
		u8 ii,jj;

		for (ii=1;ii<=CalcMatL._Row;ii++)
		{
			for (jj=1;jj<=CalcMatR._Col;jj++)
			{
				SumVal+=((*LPtr)*(*RPtr));
				LPtr++;
				RPtr++;
			}
		}

		CalcMatL.CleanRet();
		CalcMatR.CleanRet();

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

		for (ii=1;ii<=NewMat._Row;ii++)
		{
			for (jj=1;jj<=NewMat._Col;jj++)
			{
				NewMat(ii,jj,(*LPtr)+CalcValue);
				LPtr++;
			}
		}

		CalcMat.CleanRet();

		return NewMat;
	}

	//Calculate Minus
	friend Mat<Type> operator-(Mat<Type> CalcMat, Type CalcValue)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;

		for (ii=1;ii<=NewMat._Row;ii++)
		{
			for (jj=1;jj<=NewMat._Col;jj++)
			{
				NewMat(ii,jj,(*LPtr)-CalcValue);
				LPtr++;
			}
		}

		CalcMat.CleanRet();

		return NewMat;
	}

	//Calculate Multiply
	friend Mat<Type> operator*(Mat<Type> CalcMat, Type CalcValue)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;

		for (ii=1;ii<=NewMat._Row;ii++)
		{
			for (jj=1;jj<=NewMat._Col;jj++)
			{
				NewMat(ii,jj,(*LPtr)*CalcValue);
				LPtr++;
			}
		}

		CalcMat.CleanRet();

		return NewMat;
	}

	//Calculate Devide
	friend Mat<Type> operator/(Mat<Type> CalcMat, Type CalcValue)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;

		for (ii=1;ii<=NewMat._Row;ii++)
		{
			for (jj=1;jj<=NewMat._Col;jj++)
			{
				NewMat(ii,jj,(*LPtr)/CalcValue);
				LPtr++;
			}
		}

		CalcMat.CleanRet();

		return NewMat;
	}

	//Calculate Equal from Plus
	Mat<Type> & operator+=(Type CalcValue)
	{
		Type *LPtr=_Value;
		u8 ii,jj;

		for (ii=1;ii<=_Row;ii++)
		{
			for (jj=1;jj<=_Col;jj++)
			{
				this->operator ()(ii,jj,(*LPtr)+CalcValue);
				LPtr++;
			}
		}

		return *this;
	}

	//Calculate Equal from Minus
	Mat<Type> & operator-=(Type CalcValue)
	{
		Type *LPtr=_Value;
		u8 ii,jj;

		for (ii=1;ii<=_Row;ii++)
		{
			for (jj=1;jj<=_Col;jj++)
			{
				this->operator ()(ii,jj,(*LPtr)-CalcValue);
				LPtr++;
			}
		}

		return *this;
	}

	//Calculate Equal from Multiply
	Mat<Type> & operator*=(Type CalcValue)
	{
		Type *LPtr=_Value;
		u8 ii,jj;

		for (ii=1;ii<=_Row;ii++)
		{
			for (jj=1;jj<=_Col;jj++)
			{
				this->operator ()(ii,jj,(*LPtr)*CalcValue);
				LPtr++;
			}
		}

		return *this;
	}

	//Calculate Equal from Devide
	Mat<Type> & operator/=(Type CalcValue)
	{
		Type *LPtr=_Value;
		u8 ii,jj;

		for (ii=1;ii<=_Row;ii++)
		{
			for (jj=1;jj<=_Col;jj++)
			{
				this->operator ()(ii,jj,(*LPtr)/CalcValue);
				LPtr++;
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

		for (ii=1;ii<=NewMat._Row;ii++)
		{
			for (jj=1;jj<=NewMat._Col;jj++)
			{
				NewMat(ii,jj,(*LPtr)+CalcValue);
				LPtr++;
			}
		}

		CalcMat.CleanRet();

		return NewMat;
	}

	//Calculate Minus
	friend Mat<Type> operator-(Type CalcValue, Mat<Type> CalcMat)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;

		for (ii=1;ii<=NewMat._Row;ii++)
		{
			for (jj=1;jj<=NewMat._Col;jj++)
			{
				NewMat(ii,jj,(*LPtr)-CalcValue);
				LPtr++;
			}
		}

		CalcMat.CleanRet();

		return NewMat;
	}

	//Calculate Multiply
	friend Mat<Type> operator*(Type CalcValue, Mat<Type> CalcMat)
	{
		RetMat(Type,NewMat,CalcMat._Row,CalcMat._Col);
		Type *LPtr=CalcMat._Value;
		u8 ii,jj;

		for (ii=1;ii<=NewMat._Row;ii++)
		{
			for (jj=1;jj<=NewMat._Col;jj++)
			{
				NewMat(ii,jj,(*LPtr)*CalcValue);
				LPtr++;
			}
		}

		CalcMat.CleanRet();

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

		CompareMatL.CleanRet();
		CompareMatR.CleanRet();

		return NewMat;
	}

	//Compare Smaller
	friend Mat<bool> operator<(Mat<Type> CompareMatL,Mat<Type> CompareMatR)
	{
		RetMat(bool,NewMat,CompareMatL._Row,CompareMatR._Col);
		Type *LPtr=CompareMatL._Value;
		Type *RPtr=CompareMatR._Value;

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

		CompareMatL.CleanRet();
		CompareMatR.CleanRet();

		return NewMat;
	}

	//Compare Equal
	friend Mat<bool> operator==(Mat<Type> CompareMatL,Mat<Type> CompareMatR)
	{
		RetMat(bool,NewMat,CompareMatL._Row,CompareMatR._Col);
		Type *LPtr=CompareMatL._Value;
		Type *RPtr=CompareMatR._Value;

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

		CompareMatL.CleanRet();
		CompareMatR.CleanRet();

		return NewMat;
	}

	//Compare Inequal
	friend Mat<bool> operator!=(Mat<Type> CompareMatL,Mat<Type> CompareMatR)
	{
		RetMat(bool,NewMat,CompareMatL._Row,CompareMatR._Col);
		Type *LPtr=CompareMatL._Value;
		Type *RPtr=CompareMatR._Value;

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

		CompareMatL.CleanRet();
		CompareMatR.CleanRet();

		return NewMat;
	}

	//Compare No Smaller
	friend Mat<bool> operator>=(Mat<Type> CompareMatL,Mat<Type> CompareMatR)
	{
		RetMat(bool,NewMat,CompareMatL._Row,CompareMatR._Col);
		Type *LPtr=CompareMatL._Value;
		Type *RPtr=CompareMatR._Value;

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

		CompareMatL.CleanRet();
		CompareMatR.CleanRet();

		return NewMat;
	}

	//Compare No Bigger
	friend Mat<bool> operator<=(Mat<Type> CompareMatL,Mat<Type> CompareMatR)
	{
		RetMat(bool,NewMat,CompareMatL._Row,CompareMatR._Col);
		Type *LPtr=CompareMatL._Value;
		Type *RPtr=CompareMatR._Value;

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

		CompareMatL.CleanRet();
		CompareMatR.CleanRet();

		return NewMat;
	}

	//Logic AND
	friend Mat<bool> operator&(Mat<Type> BoolMatL,Mat<Type> BoolMatR)
	{
		RetMat(bool,NewMat,BoolMatL._Row,BoolMatR._Col);
		bool *ValL=BoolMatL._Value;
		bool *ValR=BoolMatR._Value;

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

		BoolMatL.CleanRet();
		BoolMatR.CleanRet();

		return NewMat;
	}

	//Logic OR
	friend Mat<bool> operator|(Mat<Type> BoolMatL,Mat<Type> BoolMatR)
	{
		RetMat(bool,NewMat,BoolMatL._Row,BoolMatR._Col);
		bool *ValL=BoolMatL._Value;
		bool *ValR=BoolMatR._Value;

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

		BoolMatL.CleanRet();
		BoolMatR.CleanRet();

		return NewMat;
	}

	//Logic XOR
	friend Mat<bool> operator^(Mat<Type> BoolMatL,Mat<Type> BoolMatR)
	{
		RetMat(bool,NewMat,BoolMatL._Row,BoolMatR._Col);
		bool *ValL=BoolMatL._Value;
		bool *ValR=BoolMatR._Value;

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

		BoolMatL.CleanRet();
		BoolMatR.CleanRet();

		return NewMat;
	}

	//Equal from Logic AND
	Mat<bool> & operator&=(Mat<bool> BoolMat)
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

		BoolMat.CleanRet();

		return *this;
	}

	//Equal from Logic OR
	Mat<bool> & operator|=(Mat<bool> BoolMat)
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

		BoolMat.CleanRet();

		return *this;
	}

	//Equal from Logic XOR
	Mat<bool> & operator^=(Mat<bool> BoolMat)
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

		BoolMat.CleanRet();

		return *this;
	}

	//////End of Comparison and Bool Interactions

};


#endif


