// File: Matrix.cpp
// Author: Matthew Leeds
// This class defines a matrix that can be used for multiplication/addition/subtraction.

#include "Matrix.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <cmath>

// This constructor allocates space for the matrix.
// If identity == true, construct the identity matrix.
Matrix::Matrix(int size, bool identity) {
    this->_size = size;
    this->_matrix = new int*[this->_size];
    for (int i = 0; i < this->_size; ++i) {
        this->_matrix[i] = new int[this->_size];
        if (identity) {
            for (int j = 0; j < this->_size; ++j)
                this->_matrix[i][j] = (i == j ? 1 : 0);
        }
    }
}

// (deep) copy constructor
Matrix::Matrix(const Matrix& other) {
    this->_size = other._size;
    this->_matrix = new int*[this->_size];
    for (int i = 0; i < this->_size; ++i) {
        this->_matrix[i] = new int[this->_size];
        for (int j = 0; j < this->_size; ++j) {
            this->_matrix[i][j] = other._matrix[i][j];
        }
    }
}

// This constructor allocates space and fills the matrix with values from a file.
// It is assumed that the file is formatted correctly, and that the 
// ifstream object is seeked to the right position before being passed to this.
Matrix::Matrix(ifstream& inFile, int size) {
    this->_size = size;
    this->_matrix = new int*[this->_size];
    for (int i = 0; i < this->_size; i++) {
        this->_matrix[i] = new int[this->_size];
    }
    // For each line, interpret everything but commas as integers
    for (int i = 0; i < size; i++) {
        string line = "";
        inFile >> line;
        stringstream ss(line);
        int j = 0;
        int num;
        while (ss >> num) {
            this->_matrix[i][j] = num;
            j++;
            if (ss.peek() == ',') {
                ss.ignore();
            }
        }
    }
}

Matrix::~Matrix() {
    for (int i = 0; i < this->_size; i++) {
        delete []this->_matrix[i];
    }
    delete []this->_matrix;
}

// returns the (n-1)x(n-1) matrix without row i or column j
Matrix* Matrix::getSubmatrix(int i, int j) {
    Matrix* sub = new Matrix(this->_size - 1, false);
    // Use separate variables for the submatrix so we don't go out of bounds.
    int subRow = 0;
    for (int thisRow = 0; thisRow < _size; ++thisRow) {
        if (thisRow == i) continue;
        int subCol = 0;
        for (int thisCol = 0; thisCol < _size; ++thisCol) {
            if (thisCol == j) continue;
            sub->_matrix[subRow][subCol] = this->_matrix[thisRow][thisCol];
            subCol++;
        }
        subRow++;
    }
    return sub;
}

// find the determinant of this if it's 3x3 or smaller
int Matrix::findDeterminant() {
    int det;
    if (this->_size == 1) {
        det = _matrix[0][0];
    } else if (this->_size == 2) {
        det = _matrix[0][0] * _matrix[1][1] - _matrix[0][1] * _matrix[1][0];
    } else if (this->_size == 3) {
        int term1 = _matrix[0][0] * _matrix[1][1] * _matrix[2][2];
        int term2 = _matrix[0][1] * _matrix[1][2] * _matrix[2][0];
        int term3 = _matrix[0][2] * _matrix[1][0] * _matrix[2][1];
        int term4 = _matrix[0][2] * _matrix[1][1] * _matrix[2][0];
        int term5 = _matrix[0][0] * _matrix[1][2] * _matrix[2][1];
        int term6 = _matrix[0][1] * _matrix[1][0] * _matrix[2][2];
        det = term1 + term2 + term3 - term4 - term5 - term6;
    } else {
        cerr << "Warning: stubbornly refusing to find the determinant of a matrix larger than 3x3." << endl;
    }
    return det;
}

// find the adjoint matrix if it's 2x2 or 3x3
// the (i,j) entry of the adjoint is (-1)^i+j * det(sub(j,i))
// where sub(j,i) is the submatrix without row j or column i
Matrix* Matrix::findAdjoint() {
    Matrix* adj = new Matrix(this->_size, false);
    if (this->_size == 2) {
        adj->_matrix[0][0] = this->_matrix[1][1]; 
        adj->_matrix[0][1] = -1 * this->_matrix[0][1];
        adj->_matrix[1][0] = -1 * this->_matrix[1][0];
        adj->_matrix[1][1] = this->_matrix[0][0];
    } else if (this->_size == 3) {
        for (int i = 0; i < this->_size; ++i) {
            for (int j = 0; j < this->_size; ++j) {
                // find the (j,i) submatrix
                Matrix* submatrix = this->getSubmatrix(j, i); 
                // find the determinant of that submatrix
                adj->_matrix[i][j] = submatrix->findDeterminant();
                // find the cofactor by multiplying the det by -1 for odd (i+j)
                adj->_matrix[i][j] *= ((i+j) % 2 == 0 ? 1 : -1);
            }
        }
    } else {
        cerr << "Warning: stubbornly refusing to find the adjoint of a matrix larger than 3x3." << endl;
    }
    return adj;
}

// Multiplies this matrix by the n by 1 matrix given, with the mod given
// and returns the newly allocated result
int* Matrix::modMultiplyVector(int* arr, int mod) {
    int n = this->_size;
    int* result = new int[n];
    for (int i = 0; i < n; ++i) {
        result[i] = 0;
        for (int j = 0; j < n; ++j) {
            result[i] += this->_matrix[i][j] * arr[j];
        }
        result[i] %= mod;
        if (result[i] < 0) result[i] += mod;
    }
    return result;
}

// multiplies this matrix in place by the scalar given, with a modulo 
void Matrix::modMultiplyScalar(int val, int mod) {
    for (int i = 0; i < this->_size; ++i) {
        for (int j = 0; j < this->_size; ++j) {
            this->_matrix[i][j] *= val;
            this->_matrix[i][j] %= mod;
            // % is division remainder not modulo
            if (this->_matrix[i][j] < 0)
                this->_matrix[i][j] += mod;
        }
    }
}

// Find the maximum magnitude value in the matrix. This can be useful for 
// determining how wide to make columns when printing it out.
int Matrix::findMaxMagnitude() const{
    int maxVal = 0;
    for (int i = 0; i < this->_size; i++) {
        for (int j = 0; j < this->_size; j++) {
            if (abs(this->_matrix[i][j]) > maxVal) {
                maxVal = abs(this->_matrix[i][j]);
            }
        }
    }
    return maxVal;
}

// This overloads the redirection operator so matrices can be printed using cout.
// Values are printed in fixed-width columns, right-aligned.
ostream& operator<<(ostream& os, const Matrix& m) {
    int maxVal = m.findMaxMagnitude();
    string maxValString;
    ostringstream convert;
    convert << maxVal;
    maxValString = convert.str();
    int extraSpace = 2;
    int fieldWidth = extraSpace + maxValString.size();
    for (int i = 0; i < m._size; i++) {
        for (int j = 0; j < m._size; j++) {
            os << right << setw(fieldWidth) << m._matrix[i][j];
        }
        os << endl;
    }
    return os;
}

// This overloads the addition operator to add two square matrices 
// by adding their respective elements.
Matrix* operator+(const Matrix& m1, const Matrix& m2) {
    int n = m1._size;
    Matrix* pSumMatrix = new Matrix(n, false);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            pSumMatrix->_matrix[i][j] = (m1._matrix[i][j] + m2._matrix[i][j]);
        }
    }
    return pSumMatrix; 
}

// This overloads the subtraction operator for (square) matrices.
Matrix* operator-(const Matrix& m1, const Matrix& m2) {
    int n = m1._size;
    Matrix* pDifferenceMatrix = new Matrix(n, false);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            pDifferenceMatrix->_matrix[i][j] = (m1._matrix[i][j] - m2._matrix[i][j]);
        }
    }
    return pDifferenceMatrix; 
}

