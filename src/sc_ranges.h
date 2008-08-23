/*
  This file is part of the SC Library.
  The SC library provides support for parallel scientific applications.

  Copyright (C) 2008 Carsten Burstedde, Lucas Wilcox.

  The SC Library is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  The SC Library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with the SC Library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SC_RANGES_H
#define SC_RANGES_H

#ifndef SC_H
#error "sc.h should be included before this header file"
#endif

/** Compute the optimal ranges of processors to talk to.
 *
 * \param [in] num_procs    Number of processors processed.
 * \param [in] procs        Array [num_procs] interpreted as booleans.
 *                          Nonzero entries need to be talked to.
 * \param [in] rank         The id of the calling process.
 *                          Will be excluded from the ranges.
 * \param [in] first_peer   First processor to be considered.
 * \param [in] last_peer    Last processor to be considered (inclusive).
 * \param [in] num_ranges   The number of ranges to compute.
 * \param [in,out] ranges   Array [2 * num_ranges] that will be filled
 *                          with beginning and ending procs (inclusive)
 *                          that represent each range.  Values of -1
 *                          indicate that the range is not needed.
 * \return                  Returns the number of filled ranges.
 */
int                 sc_ranges_compute (int num_procs, int *procs, int rank,
                                       int first_peer, int last_peer,
                                       int num_ranges, int *ranges);

/** Compute the globally optimal ranges of processors.
 *
 * \param [in] mpicomm      MPI Communicator for Allreduce and Allgather.
 * \param [in] procs        Same as in sc_ranges_compute ().
 * \param [in,out] inout1
 *     On input, first_peer as in sc_ranges_compute ().
 *     On output, global maximum of peer counts.
 * \param [in,out] inout2
 *     On input, last_peer as in sc_ranges_compute ().
 *     On output, global maximum of ranges.
 * \param [in] num_ranges   Same as in sc_ranges_compute ().
 * \param [in] ranges       Same as in sc_ranges_compute ().
 * \param [out] global_ranges
 *     If not NULL, will be allocated and filled with everybody's ranges.
 *     Size will be 2 * inout2 * num_procs.  Must be freed with SC_FREE ().
 * \return                  Returns the number of locally filled ranges.
 */
int
sc_ranges_adaptive (MPI_Comm mpicomm, int *procs, int *inout1, int * inout2,
                    int num_ranges, int *ranges, int **global_ranges);

/** Compute global statistical information on the ranges.
 *
 * \param [in] package_id       Registered package id or -1.
 * \param [in] log_priority     Priority to use for logging.
 */
void                sc_ranges_statistics (int package_id, int log_priority,
                                          MPI_Comm mpicomm, int num_procs,
                                          int *procs, int rank,
                                          int num_ranges, int *ranges);

#endif /* !SC_RANGES_H */