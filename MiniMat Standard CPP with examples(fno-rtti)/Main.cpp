/*
*File Name: Main.cpp
*File Creation Date: April 17th 2017
*File Final Revised Date: May 7th 2017
*Author: Jiamin Wang
*Description: Example Code for MiniMat
*/

#include "MiniMat.hpp"

int main()
{
	//Example 1: Define a Matrix and Assign Element Values
	cout<<">>Example 1:"<<endl<<endl;
	TmpMat(double,TMat1,5,5);
	TMat1>>0		<<1,2,3,2,2,
					  4,7,2,3,2,
					  7,3,5,4,3,
					  3,2,4,1,2,
					  1,3,5,7,4;
	cout<<"The Original Base Matrix:";
	TMat1.Print();
	TMat1>>0
		 >>-1>>3	<<8;
	cout<<"The Modified Base Matrix:";
	TMat1.Print();
	cout<<endl<<">>End"<<endl<<endl;


	//Example 2: Calculate Sum, Max, Min, Negative and Absolution of the matrix
	cout<<">>Example 2:"<<endl<<endl;
	cout<<"Sum: "<<Sum(TMat1)<<endl;
	cout<<"Max: "<<Max(TMat1)<<endl;
	cout<<"Min: "<<Min(TMat1)<<endl;
	cout<<"Determinant: "<<Det(TMat1)<<endl;
	TMat1=-TMat1;
	cout<<"Matrix of Negative Elements:";
	TMat1.Print();
	cout<<"Max of Negative Matrix: "<<Max(TMat1)<<endl;
	cout<<"Min of Negative Matrix: "<<Min(TMat1)<<endl;
	TMat1=Abs(TMat1);
	cout<<"Matrix of Absolution of Elements:";
	TMat1.Print();
	cout<<endl<<">>End"<<endl<<endl;


	//Example 3: Extraction of Partial Matrix & Calculation of Norm, Point Product and Cross Product of Vectors
	cout<<">>Example 3:"<<endl<<endl;
	MirTmpMat(double,TVec1,3,1,~TMat1(3,3,1,3));//Note the transpose sign "~" here
	MirTmpMat(double,TVec2,3,1,TMat1(2,4,4,4));
	TmpMat(double,TVec3,3,1);
	cout<<"Vector 1 Extracted form the Base Matrix ";
	TVec1.Print();
	cout<<"Vector 2 Extracted form the Base Matrix ";
	TVec2.Print();
	cout<<"Norm: "<<Norm(TVec1)<<endl;
	cout<<"Point Product: "<<Point(TVec1,TVec2)<<endl;
	cout<<"Cross Product:";
	TVec3=Cross(TVec1,TVec2);
	TVec3.Print();
	cout<<"Verification of Cross Product with Vector 1: "<<Point(TVec1,TVec3)<<endl;
	cout<<"Verification of Cross Product with Vector 2: "<<Point(TVec2,TVec3)<<endl;
	cout<<endl<<">>End"<<endl<<endl;

	//Example 4: Swap Row/Column & Inverse of a Matrix
	cout<<">>Example 4:"<<endl<<endl;
	MirTmpMat(double,TMat2,5,5,TMat1);
	cout<<"Modified Base Matrix of Swapped Row:";
	TMat2.SwapRow(2,3);
	TMat2.Print();
	cout<<"Modified Base Matrix of Swapped Column:";
	TMat2=TMat1;
	TMat2.SwapCol(2,3);
	TMat2.Print();
	cout<<"Inverse of Base Matrix:";
	TMat2=!TMat1;
	TMat2.Print();
	cout<<"Verification of Inverse Matrix:";
	TMat2=TMat2*TMat1;
	TMat2.Print();
	cout<<endl<<">>End"<<endl<<endl;

	//Example 5: Complicated Calculation of Plus, Minus, Multiply, Devide, Transpose and Submatrix between Matrixes and Single Values;
	cout<<">>Example 5:"<<endl<<endl;
	MirTmpMat(double,TCalc1,4,4,TMat1.Sub(2,1));
	MirTmpMat(double,TCalc2,4,4,TMat1.Sub(3,4));
	MirTmpMat(double,TCalc3,4,4,TMat1.Sub(5,3));
	cout<<"Matrix for Calculation 1: ";
	TCalc1.Print();
	cout<<"Matrix for Calculation 2: ";
	TCalc2.Print();
	cout<<"Matrix for Calculation 3: ";
	TCalc3.Print();
	TmpMat(double,TCalcResult,4,4);
	TCalcResult=0;
	TCalcResult+=5*(TCalc1+TCalc2)*((~TCalc3)/4+2)-0.75-TCalc3*TCalc2;
	cout<<"Result of Above Calculation: ";
	TCalcResult.Print();
	cout<<endl<<">>End"<<endl<<endl;

	//Example 6: Boolean Calculations
	cout<<">>Example 6:"<<endl<<endl;
	TmpMat(bool,TBool1,4,4);
	TmpMat(bool,TBool2,4,4);
	TmpMat(bool,TBool3,4,4);
	TmpMat(bool,TBool4,4,4);
	cout<<"Comparision of Element between TCalc 1 and TCalc 2:";
	TBool1=(TCalc1>TCalc2);
	TBool1.Print();
	cout<<"Comparision of Element between TCalc 1 and TCalc 3:";
	TBool2=(TCalc1<=TCalc3);
	TBool2.Print();
	cout<<"Comparision of Element between TCalc 2 and TCalc 3:";
	TBool3=(TCalc2==TCalc3);
	TBool3.Print();
	cout<<"Logic AND between TBool 1 and TBool 2:";
	TBool4=(TBool1&TBool2);
	TBool4.Print();
	cout<<"Logic OR between TBool 1 and TBool 3:";
	TBool4=(TBool1|TBool3);
	TBool4.Print();
	cout<<"Logic XOR between TBool 2 and TBool 3:";
	TBool2^=TBool3;
	TBool2.Print();
	cout<<"Negative of TBool2";
	TBool2=TBool2.Neg();
	TBool2.Print();
	cout<<endl<<">>End"<<endl<<endl;

	//Example 7: Heap and Temporary Matrix (Return Mat)
	cout<<">>Example 7:"<<endl;
	MiniMat_HeapScope.Print();
	RetMat(double,TTemp1,5,5);
	TTemp1=TMat1;
	cout<<"The Temporary Matrix:";
	TTemp1.Print();
	MiniMat_HeapScope.Print();
	cout<<"Temporary Matrix Cleaned Here"<<endl;
	TTemp1.CleanRet();
	//TMat1=TTemp1;//This command will also clean TTemp1 (See Note in the Header);
	MiniMat_HeapScope.Print();
	cout<<endl<<">>End"<<endl<<endl;
}
