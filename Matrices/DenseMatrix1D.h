//
//  DenseMatrix1D.h
//
//  Created by Ali Hajimirza on 6/11/13.
//  Copyright (c) 2013 Ali Hajimirza. All rights reserved.
//

#ifndef _DenseMatrix1D_h
#define _DenseMatrix1D_h

#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include "MatrixExceptions.h"

#ifdef ARPACK
#include "dsmatrxa.h"
#include "ardsmat.h"
#include "ardssym.h"
#include "lsymsol.h"
#endif

template <typename T>
class DenseMatrix1D;

template <typename T>
std::ostream& operator<< (std::ostream&, const DenseMatrix1D<T>&);

template <typename T>
class DenseMatrix1D
{
private:
    static const int _DEFAULT_MATRIX_SIZE;
    static const  T _DEFAULT_MATRIX_ENTRY;
    size_t _getArrSize() const;
    void _initilalizeMatrix(bool);
    void _copy(const DenseMatrix1D<T>&);
    
protected:
    int _rows;
    int _cols;
    T* _edges;
    
    
public:
    /**************
     *Constructors*
     **************/
    DenseMatrix1D(bool);
    DenseMatrix1D(int, int,bool);
    DenseMatrix1D(const DenseMatrix1D<T>&);
    DenseMatrix1D(const std::string&);
    
    /************
     *Destructor*
     ************/
    virtual ~DenseMatrix1D();
    
    /***********
     *ACCESSORS*
     ***********/
    bool isSquare() const;
    bool isSymmetric() const;
    int getNumberOfRows();
    int getSparseFormSize();
    int getNumberOfColumns();
    std::vector<T> getSumOfRows();
    std::vector<T> getTopEigenVector();
    std::vector<int> getNeighbors(int vertex);
    DenseMatrix1D<T> getScatteredSelection(const std::vector<int>& vec_A, const std::vector<int> vec_B);
    
    /**********
     *MUTATORS*
     **********/
    void insert(int, int, T);
    
    /**********
     *OPERATORS*
     **********/
    T operator()(int,int) const;
    void operator()(int,int,T);
    DenseMatrix1D<T> kron(const DenseMatrix1D<T>&); /* WORKS SHOULD BE CHANGED */
    bool operator==(const DenseMatrix1D<T>&);
    DenseMatrix1D<T>& operator= (const DenseMatrix1D<T>&);
    DenseMatrix1D<T> diagonalVectorTimesMatrix(const std::vector<T>&);
    DenseMatrix1D<T> MatrixTimesDiagonalVector(const std::vector<T>&);
    friend std::ostream& operator<< <> (std::ostream& stream, const DenseMatrix1D<T>& DenseMatrix1D);
    
    
    /* WILL IMPLEMENT IF I HAD TIME
     DenseMatrix1D<T> operator* (const DenseMatrix1D<T>&);
     DenseMatrix1D<T> operator+ (const DenseMatrix1D<T>&);
     DenseMatrix1D<T> operator- (const DenseMatrix1D<T>&);
     DenseMatrix1D<T> operator* (T);
     DenseMatrix1D<T> operator+ (T);
     DenseMatrix1D<T> operator- (T);
     void operator*= (const DenseMatrix1D<T>&);
     void operator+= (const DenseMatrix1D<T>&);
     void operator-= (const DenseMatrix1D<T>&);
     void operator*= (T);
     void operator+= (T);
     void operator-= (T);
     */
    
};

//==========================================================CONSTANTS============================================================
template <typename T>
const int DenseMatrix1D<T>::_DEFAULT_MATRIX_SIZE = 1;
template <typename T>
const T DenseMatrix1D<T>::_DEFAULT_MATRIX_ENTRY = 1;
//==========================================================CONSTRUCTORS============================================================
template <typename T>
inline DenseMatrix1D<T>::DenseMatrix1D(bool fill = false)
{
    this->_rows = _DEFAULT_MATRIX_SIZE;
    this->_cols = _DEFAULT_MATRIX_SIZE;
    _initilalizeMatrix(fill);
}

template<typename T>
inline DenseMatrix1D<T>::DenseMatrix1D(const std::string& file_path)
{
    int tmp_x;
    int tmp_y;
    std::ifstream file_reader;
    file_reader.open(file_path.c_str());
    
    if(file_reader.fail())
    {
        file_reader.close();
        throw FileDoesNotExistException(file_path.c_str());
    }
    
    file_reader >> this->_rows;
    file_reader >> this->_cols;
    
    _initilalizeMatrix();
    file_reader >> tmp_x;      //skip the line number
    
    while (!file_reader.eof())
    {
        file_reader >> tmp_x;
        file_reader >> tmp_y;
        (*this)(tmp_x - 1 ,tmp_y - 1, 1);
    }
    
    file_reader.close();
}

template <typename T>
inline DenseMatrix1D<T>::DenseMatrix1D(int rows, int cols, bool fill)
{
    this->_rows = rows;
    this->_cols = cols;
    _initilalizeMatrix(fill);
}

template <typename T>
inline DenseMatrix1D<T>::DenseMatrix1D(const DenseMatrix1D<T>& matrix)
{
    _copy(matrix);
}

//==========================================================DESTRUCTOR==============================================================
template <typename T>
inline DenseMatrix1D<T>::~DenseMatrix1D()
{
    if (this->_edges != NULL)
    {
        delete [] this->_edges;
    }
}

//===========================================================ACCESSORS===============================================================
/*
 * Returns the number of the rows.
 */
template <typename T>
inline int DenseMatrix1D<T>::getNumberOfRows()
{
    return this->_rows;
}

/*
 * Returns the number of the columns.
 */
template <typename T>
inline int DenseMatrix1D<T>::getNumberOfColumns()
{
    return this->_cols;
}

/*
 * Returns true if the matrix is square (i.e rows == cols)
 */
template <typename T>
inline bool DenseMatrix1D<T>::isSquare() const
{
    return (this->_rows == this->_cols);
}

/*
 * Returns true if the matrix is symmetric (i.e values at i, j are equal to values at j, i)
 */
template <typename T>
inline bool DenseMatrix1D<T>::isSymmetric() const
{
    //sym. matrix has to be square
    if (this->_rows != this->_cols)
    {
        return false;
    }
    
    //chaking for entries to be equal
    for(int i = 0; i < this->_rows; i++)
    {
        for(int j = 0; j < this->_cols; j++)
        {
            if (this->_edges[(i * this->_cols) + j] != this->_edges[(j * this->_cols) + i])
            {
                return false;
            }
        }
    }
    
    return true;
}

/*
 * Returns the frobenius norm.
 */
template <typename T>
T DenseMatrix1D<T>::getFrobNorm() const
{
    // T ret_val=0;

    // for(int i=0; i < this->_rows; i++)
    // {
    //     for(int j=0; j < this->_cols; j++)
    //     {
    //         ret_val+= this->_edges[i][j] * this->_edges[i][j];
    //     }
    // }
    // return ret_val;
}

/*
 * Returns a std::vector<SparseElement<T>> of SparseElement objects that contain the i, j and value of the none-zero edges.
 */
template <typename T>
inline std::vector<SparseElement<T> > DenseMatrix1D<T>::getSparseForm() const
{    
    std::vector<SparseElement<T> > sparse_form;
    for(int i = 0; i < this->_getArrSize(); i++)
    {
        if(this->_edges[i][j] != 0)
        {
            sparse_form.push_back(SparseElement<T>(i,j, this->_edges[i][j]));
        }
    }
    return sparse_form;
}

/*
 * Returns a std::vector<int> of the elements with value of 1 in a columns.
 * @pram int vertex
 */
template <typename T>
inline std::vector<int> DenseMatrix1D<T>::getNeighbors(int vertex) const
{
  // std::vector<int> neighbors;
  // for(int i = 0; i < this->_rows; i++)
  // {
  //   if(this->_edges[i][vertex] == 1)
  //   {
  //     neighbors.push_back(i);
  //   }
  // }
  // return neighbors;
} 

/*
 * Returns a DenseMatrix2D that has the rows that are marked 1 in vec_A, columns that are marked 1 in vec_B
 * @pram std::vector<int>: vector of 0's and 1's for row selection
 * @pram std::vector<int>: vector of 0's and 1's for column selection
 */
template <typename T>
inline DenseMatrix1D<T> DenseMatrix1D<T>::getScatteredSelection(const std::vector<int>& vec_A, const std::vector<int> vec_B)
{
    int num_in_A = 0;
    for (int i=0; i< vec_A.size(); i++)
    {
        if (vec_A[i] == 1)
        {
            num_in_A++;
        }
    }
    int num_in_B = 0;
    for (int i=0; i < vec_B.size(); i++)
    {
        if( vec_B[i] == 1)
        {
            num_in_B++;
        }
    }
    //Initializing and allocating the product matrix
    DenseMatrix1D<T> res_matrix(num_in_A, num_in_B);
    
    int counter = 0;
    
    for (int i=0; i < vec_A.size(); i++)
    {
        for(int j=0; j < vec_B.size(); j++)
        {
            if ( vec_A[i] == 1 && vec_B[j] ==1)
            {
                res_matrix._edges[counter] = (*this)(i,j);
                counter++;
            }
        }
    }
    return res_matrix;
}

template <typename T>
inline std::vector<int> DenseMatrix1D<T>::getNeighbors(int vertex)
{
    std::vector<int> neighbors;
    
    for(int i = 0; i < this->getNumberOfRows(); i++)
    {
        if(this->_edges[(i * this->_cols) + vertex] == 1)
        {
            neighbors.push_back(i);
        }
    }
    
    return neighbors;
}

/*
 * Returns a std::vector<T> that contains the values of the eigenvector associated to the largest eigenvalue
 */
template <typename T>
inline std::vector<T> DenseMatrix1D<T>::getTopEigenVector()
{
#ifdef ARPACK
    int arr_size = (0.5 * this->_rows * (this->_rows+1));
    T sym_edges[arr_size];
    int counter = 0;
    
    for(int i = 0; i < this->_rows; i++)
    {
        for(int j = i; j < this->_cols; j++)
        {
            sym_edges[counter] = (*this)(i,j);
            counter++;
        }
    }
    
    ARdsSymMatrix<T> ARMatrix(this->_rows, sym_edges, 'L');
    ARluSymStdEig<T> eigProb(1, ARMatrix, "LM", 10);
    eigProb.FindEigenvectors();
    
    std::vector<T> eigen_vec (eigProb.GetN());
    for (int i=0; i < eigProb.GetN() ; i++)
    {
        eigen_vec[i] = eigProb.Eigenvector(0 ,i);
    }
    
    return eigen_vec;
#endif
}

/*
 * returns a vector of type T where each entry corresponds to sum of entries in a matrix row.
 * Delete this object after usage.
 * @return std::vector<T>
 */
template <typename T>
inline std::vector<T> DenseMatrix1D<T>::getSumOfRows()
{
    std::vector<T> sum_vector(this->_rows);
    for(int i = 0; i < this->_getArrSize(); i++)
    {
        sum_vector[(i / this->_rows)] += this->_edges[i];
    }
    
    return sum_vector;
}

//===========================================================MUTATORS================================================================

//==========================================================OPERATORS================================================================
template <typename T>
inline DenseMatrix1D<T> DenseMatrix1D<T>::kron(const DenseMatrix1D<T>& matrix)
{
    // checking for matrices to be square
    if (!this->isSquare() || !matrix.isSquare())
    {
        throw NotASquareMatrixException();
    }
    
    //Initializing and allocating the product matrix
    int prod_size = this->_rows * matrix._rows;
    DenseMatrix1D<T> prod_matrix(prod_size, prod_size);
    
    /*
     *  Calculating the kronecker product:
     *  The indices of the product matrix is calculated by:
     *      i = (i_outer*size) + i_inner
     *      j = (j_outer*size) + j_inner
     */
    for (int i_outer = 0; i_outer < this->_rows; i_outer++)
    {
        for (int j_outer=0; j_outer < this->_cols; j_outer++)
        {
            for(int i_inner=0; i_inner < matrix._rows; i_inner++)
            {
                for(int j_inner=0; j_inner < matrix._cols; j_inner++)
                {
                    prod_matrix.insert((i_outer*matrix._rows) + i_inner,
                                       (j_outer*matrix._cols) + j_inner,
                                       matrix(i_inner, j_inner) * (*this)(i_outer , j_outer));
                }
            }
        }
    }
    
    //new method to take more advantage of 1d array and possibly faster, the assignment needs some thinking
    
    //    DenseMatrix1D<T>* kron_prod = new DenseMatrix1D<T>(prod_size, prod_size);
    //    int counter = 0;
    //    for (int i = 0; i < this->_getArrSize(); i++)
    //    {
    //        for (int j = 0; j < matrix._getArrSize(); j++)
    //        {
    //            kron_prod->_edges[counter++] = this->_edges[i] * matrix._edges[j];
    //        }
    //    }
    //    std::cout << (*kron_prod) << std::endl;
    //    std::cout << ((*kron_prod) == (*prod_matrix)) << std::endl;
    
    return prod_matrix;
}

/*
 * Returns a DenseMatrix2D<T> of a vector that contains the diagonal values of a diagonal matrix times this matrix.
 * @pram std::vector<T> diagonal entires of a diagonal matrix
 */
template <typename T>
inline DenseMatrix1D<T> DenseMatrix1D<T>::diagonalVectorTimesMatrix(const std::vector<T>& vec)
{
    if(this->_rows != vec.size())
    {
        throw DimensionMismatchException();
    }
    
    DenseMatrix1D<T> ret_matrix(*this);
    for(int i = 0; i < this->_getArrSize(); i++)
    {
        ret_matrix._edges[i] = vec[i/this->_rows] * this->_edges[i];
    }
    
    return ret_matrix;
}

/*
 * Returns a DenseMatrix2D<T> of the product of this matrix a vector that contains the diagonal values of a diagonal matrix.
 * @pram std::vector<T> diagonal entires of a diagonal matrix
 */
template <typename T>
inline DenseMatrix1D<T> DenseMatrix1D<T>::matrixTimesDiagonalVector(const std::vector<T>& vec)
{
    if(this->_cols != vec.size())
    {
        throw DimensionMismatchException();
    }
    
    DenseMatrix1D<T> ret_matrix(*this);
    for(int i = 0; i < this->_getArrSize(); )
    {
        for(int j = 0; j < vec.size(); j++)
        {
            ret_matrix._edges[i] = this->_edges[i] * vec[j];
            i++;
        }
    }
    return ret_matrix;
}

/*
 * Returns a DenseMatrix2D<T> of the transpose of this.
 */
template <typename T>
DenseMatrix1D<T> DenseMatrix1D<T>::transpose() const
{
    DenseMatrix2D<T> ret_matrix(this->_rows,this->_cols);

    for(int i=0;i<this->_rows;i++)
    {
        for(int j=0;j<this->_cols;j++)
        {
            ret_matrix._edges[j][i] = this->_edges[i][j];
        }
    }
    return ret_matrix;
}

/*
 * overloaded ostream operator for printing a matrix
 * @pram: std::ostream 
 * @pram: DenseMatrix2D<T>
 */
template <typename T>
inline std::ostream& operator<<(std::ostream& stream, const DenseMatrix1D<T>& matrix)
{
    stream<< "Size: " << matrix._rows << "*" << matrix._cols << '\n';
    for (int i = 1; i <= matrix._getArrSize(); i++)
    {
        stream << matrix._edges[i-1] << ' ';
        if ( (i % matrix._rows) == 0)
        {
            stream << "\n";
        }
    }
    stream << "\n\n\n";
    return stream;
}

/*
 * Overloaded () operator for accessing and changing the values inside a matrix
 * @pram: int i
 * @pram: int j
 */
template <typename T>
inline T& DenseMatrix1D<T>::operator()(int i, int j)
{
    return this->_edges[(i * this->_cols) + j];
}

/*
 * Returns a DenseMatrix2D that is product of this and other_matrix
 * @pram: DenseMatrix2D<T> 
 */
template <typename T>
inline DenseMatrix1D<T> DenseMatrix1D<T>::operator*(const DenseMatrix2D<T>& other_matrix) const
{
    // DenseMatrix2D<T> ret_matrix(this->_rows,this->_cols);
    // T ret_val;
    // for(int i = 0; i < this->_rows;i++)
    // {
    //     for(int j = 0; j < other_matrix._cols;j++)
    //     {
    //         ret_val = 0;
    //         for(int k = 0; k < this->_cols;k++)
    //         {
    //            ret_val += this->_edges[i][k] * other_matrix._edges[k][j];
    //         }
    //         ret_matrix._edges[i][j] = ret_val;
    //     }
    // }
  
    // return ret_matrix;
}

/*
 * Returns a DenseMatrix2D that is the difference of this and other_matrix
 * @pram: DenseMatrix2D<T> 
 */
template <typename T>
inline DenseMatrix1D<T> DenseMatrix1D<T>::operator-(const DenseMatrix2D<T>& other_matrix) const
{
    DenseMatrix1D<T> ret_matrix(this->_rows,this->_cols);
    for(int i = 0; i < this->_getArrSize() ;i++)
    {   
        ret_matrix._edges[i] = this->_edges[i] - other_matrix._edges[i];
    }
    return ret_matrix;
}

/*
 * Returns a DenseMatrix2D that is the sum of this and other_matrix
 * @pram: DenseMatrix2D<T> 
 */
template <typename T>
inline DenseMatrix1D<T> DenseMatrix1D<T>::operator+(const DenseMatrix2D<T>& other_matrix) const
{
    DenseMatrix1D<T> ret_matrix(this->_rows,this->_cols);
    for(int i = 0; i < this->_getArrSize() ;i++)
    {   
        ret_matrix._edges[i] = this->_edges[i] + other_matrix._edges[i];
    }
    return ret_matrix;
}

/*
 * Overloaded = operator copies the content of another matrix to this
 * @pram: DenseMatrix2D<T> 
 */
template <typename T>
inline DenseMatrix1D<T>& DenseMatrix1D<T>::operator=(const DenseMatrix1D<T>& matrix)
{
    if (this->_edges != NULL)
    {
        delete [] this->_edges;
    }
    _copy(matrix);
}

/*
 * Overloaded == operator to compare two matrices
 * @pram: DenseMatrix2D<T> 
 */
template <typename T>
inline bool DenseMatrix1D<T>::operator==(const DenseMatrix1D<T>& matrix)
{
    // checking for dimension equality
    if ((this->_cols != matrix._cols) || (this->_rows != matrix._rows))
    {
        return false;
    }
    
    //checking for entry equality
    for (int i = 0; i < this->_getArrSize(); i++)
    {
        if ( this->_edges[i] != matrix._edges[i])
        {
            return false;
        }
    }
    
    return true;
}
    
//===========================================================PRIVATE=================================================================
/*
 * Make a deep copy of a matrix object
 * @pram: DenseMatrix2D<T> 
 */
template <typename T>
inline void DenseMatrix1D<T>::_copy(const DenseMatrix1D<T>& matrix)
{
    this->_rows = matrix._rows;
    this->_cols = matrix._cols;
    _initilalizeMatrix();
    memcpy(this->_edges, matrix._edges, this->_getArrSize() * sizeof(T));
}

/*
 * makes a 2D array of size _rows*_cols
 * @pram: bool fill: if true: initialize values to 0.
 */  
template <typename T>
inline void DenseMatrix1D<T>::_initilalizeMatrix(bool fill)
{
    try
    {
        if (fill)
        {
            this->_edges = new T[this->_cols]();
        }
        else
        {
            this->_edges = new T[this->_cols];
        }
    }
    catch (std::bad_alloc& e)
    {
        throw OutOfMemoryException();
    }
}

template<typename T>
inline size_t DenseMatrix1D<T>::_getArrSize() const
{
    return (this->_rows * this->_cols);
}
    
//===================================================================================================================================
    
    
#endif
    