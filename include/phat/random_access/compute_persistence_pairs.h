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

#include <phat/common/persistence_pairs.h>
#include <phat/common/dualize.h>
#include <phat/random_access/reducers/twist.h>
#include <phat/random_access/boundary_matrix.h>

namespace phat { namespace random_access {

    template< typename ReductionAlgorithm, typename Representation >
    void compute_persistence_pairs( common::persistence_pairs& pairs, boundary_matrix< Representation >& boundary_matrix ) {
        ReductionAlgorithm reduce;
        reduce( boundary_matrix );
        pairs.read_off_pairs( boundary_matrix );
    }
    
    template< typename ReductionAlgorithm, typename Representation >
    void compute_persistence_pairs_dualized( common::persistence_pairs& pairs, boundary_matrix< Representation >& boundary_matrix ) {
        common::dualize( boundary_matrix );
        compute_persistence_pairs< ReductionAlgorithm >( pairs, boundary_matrix );
        common::dualize_persistence_pairs( pairs, boundary_matrix.get_num_cols() );
    }
    
    template< typename Representation >
    void compute_persistence_pairs( common::persistence_pairs& pairs, boundary_matrix< Representation >& boundary_matrix ) {
        compute_persistence_pairs< twist_reduction >( pairs, boundary_matrix );
    }
    
    
    template< typename Representation >
    void compute_persistence_pairs_dualized( common::persistence_pairs& pairs, boundary_matrix< Representation >& boundary_matrix ) {
        compute_persistence_pairs_dualized< twist_reduction >( pairs, boundary_matrix );
    }
} }