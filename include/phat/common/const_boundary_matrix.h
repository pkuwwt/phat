/*  Copyright 2013 IST Austria
    Contributed by: Ulrich Bauer, Michael Kerber, Jan Reininghaus

    This file is part of PHAT.

    PHAT is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    PHAT is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with PHAT.  If not, see <http://www.gnu.org/licenses/>. */

#pragma once

#include <phat/common/basic_types.h>
#include <phat/random_access/representations/bit_tree_pivot.h>

// interface class for the main data structure -- implementations of the interface can be found in ./representations
namespace phat { namespace common {
    template< class Representation = bit_tree_pivot >
    class const_boundary_matrix
    {
        
    protected:
        Representation rep;

    // interface functions -- actual implementation and complexity depends on chosen @Representation template
    public:
        // get overall number of columns in boundary_matrix
        index get_num_cols() const { return rep._get_num_cols(); }

        // get dimension of given index
        dimension get_dim( index idx ) const { return rep._get_dim( idx ); }

        // replaces content of @col with boundary of given index
        void get_col( index idx, column& col  ) const { col.clear(); rep._get_col( idx, col ); }

        // true iff boundary of given column is empty
        bool is_empty( index idx ) const { return rep._is_empty( idx ); }

        // largest index of given column (new name for lowestOne())
        index get_max_index( index idx ) const { return rep._get_max_index( idx ); }

        // syncronizes all internal data structures -- has to be called before and after any multithreaded access!
        void sync() { rep._sync(); }

        // has to be called initially to prepare internal data structures
        void init( index num_cols ) { rep._init( num_cols ); }

    // info functions -- independent of chosen 'Representation'
    public:
        // maximal dimension
        dimension get_max_dim() const {
            dimension cur_max_dim = 0;
            for( index idx = 0; idx < get_num_cols(); idx++ )
                cur_max_dim = get_dim( idx ) > cur_max_dim ? get_dim( idx ) : cur_max_dim;
            return cur_max_dim;
        }

        // number of nonzero rows for given column @idx
        index get_num_rows( index idx ) const {
            column cur_col;
            get_col( idx, cur_col );
            return cur_col.size();
        }

        // maximal number of nonzero rows of all columns
        index get_max_col_entries() const {
            index max_col_entries = -1;
            const index nr_of_columns = get_num_cols();
            for( index idx = 0; idx < nr_of_columns; idx++ )
                max_col_entries = get_num_rows( idx ) > max_col_entries ? get_num_rows( idx ) : max_col_entries;
            return max_col_entries;
        }

        // maximal number of nonzero cols of all rows
        index get_max_row_entries() const {
            size_t max_row_entries = 0;
            const index nr_of_columns = get_num_cols();
            std::vector< std::vector< index > > transposed_matrix( nr_of_columns );
            column temp_col;
            for( index cur_col = 0; cur_col < nr_of_columns; cur_col++ ) {
                get_col( cur_col, temp_col );
                for( index idx = 0; idx < (index)temp_col.size(); idx++)
                    transposed_matrix[ temp_col[ idx ]  ].push_back( cur_col );
            }
            for( index idx = 0; idx < nr_of_columns; idx++ )
                max_row_entries = transposed_matrix[ idx ].size() > max_row_entries ? transposed_matrix[ idx ].size() : max_row_entries;
            return max_row_entries;
        }

        // overall number of entries in the matrix
        index get_num_entries() const {
            index number_of_nonzero_entries = 0;
            const index nr_of_columns = get_num_cols();
            for( index idx = 0; idx < nr_of_columns; idx++ )
                number_of_nonzero_entries += get_num_rows( idx );
            return number_of_nonzero_entries;
        }
    
    // operators / constructors
    public:
        const_boundary_matrix() {};

        template< typename OtherRepresentation >
        bool operator==( const const_boundary_matrix< OtherRepresentation >& other_boundary_matrix ) const {
            const index number_of_columns = this->get_num_cols();

            if( number_of_columns != other_boundary_matrix.get_num_cols() )
                return false;

            column temp_col;
            column other_temp_col;
            for( index idx = 0; idx < number_of_columns; idx++ ) {
                this->get_col( idx, temp_col );
                other_boundary_matrix.get_col( idx, other_temp_col );
                if( temp_col != other_temp_col || this->get_dim( idx ) != other_boundary_matrix.get_dim( idx ) )
                    return false;
            }
            return true;
        }

        template< typename OtherRepresentation >
        bool operator!=( const const_boundary_matrix< OtherRepresentation >& other_boundary_matrix ) const {
            return !( *this == other_boundary_matrix );
        }

    // I/O -- independent of chosen 'Representation'
    public:
        template< typename index_type, typename dimemsion_type >
        void save_vector_vector( std::vector< std::vector< index_type > >& output_matrix, std::vector< dimemsion_type >& output_dims ) { 
            const index nr_of_columns = get_num_cols();
            output_matrix.resize( nr_of_columns );
            output_dims.resize( nr_of_columns );
            column temp_col;
            for( index cur_col = 0; cur_col <  nr_of_columns; cur_col++ ) {
                output_dims[ cur_col ] = (dimemsion_type)get_dim( cur_col );
                get_col( cur_col, temp_col );
                index num_rows = temp_col.size();
                output_matrix[ cur_col ].clear();
                output_matrix[ cur_col ].resize( num_rows );
                for( index cur_row = 0; cur_row <  num_rows; cur_row++ )
                    output_matrix[ cur_col ][ cur_row ] = (index_type)temp_col[ cur_row ];
            }
        }

        // Saves the boundary_matrix to given file in ascii format
        // Format: each line represents a column, first number is dimension, other numbers are the content of the column
        bool save_ascii( std::string filename ) {
            std::ofstream output_stream( filename.c_str() );
            if( output_stream.fail() )
                return false;

            const index nr_columns = this->get_num_cols();
            column tempCol;
            for( index cur_col = 0; cur_col < nr_columns; cur_col++ ) {
                output_stream << (int64_t)this->get_dim( cur_col );
                this->get_col( cur_col, tempCol );
                for( index cur_row_idx = 0; cur_row_idx < (index)tempCol.size(); cur_row_idx++ )
                    output_stream << " " << tempCol[ cur_row_idx ];
                output_stream << std::endl;
            }

            output_stream.close();
            return true;
        }

        // Saves the boundary_matrix to given file in binary format 
        // Format: nr_columns % dim1 % N1 % row1 row2 % ...% rowN1 % dim2 % N2 % ...
        bool save_binary( std::string filename ) {
            std::ofstream output_stream( filename.c_str(), std::ios_base::binary | std::ios_base::out );
            if( output_stream.fail() )
                return false;

            const int64_t nr_columns = this->get_num_cols();
            output_stream.write( (char*)&nr_columns, sizeof( int64_t ) );
            column tempCol;
            for( index cur_col = 0; cur_col < nr_columns; cur_col++ ) {
                int64_t cur_dim = this->get_dim( cur_col );
                output_stream.write( (char*)&cur_dim, sizeof( int64_t ) );
                this->get_col( cur_col, tempCol );
                int64_t cur_nr_rows = tempCol.size();
                output_stream.write( (char*)&cur_nr_rows, sizeof( int64_t ) );
                for( index cur_row_idx = 0; cur_row_idx < (index)tempCol.size(); cur_row_idx++ ) {
                    int64_t cur_row = tempCol[ cur_row_idx ];
                    output_stream.write( (char*)&cur_row, sizeof( int64_t ) );
                }
            }

            output_stream.close();
            return true;
        }
    };
} }
