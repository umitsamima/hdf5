/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright by the Board of Trustees of the University of Illinois.         *
 * All rights reserved.                                                      *
 *                                                                           *
 * This file is part of HDF5.  The full HDF5 copyright notice, including     *
 * terms governing use, modification, and redistribution, is contained in    *
 * the files COPYING and Copyright.html.  COPYING can be found at the root   *
 * of the source code distribution tree; Copyright.html can be found at the  *
 * root level of an installed copy of the electronic HDF5 document set and   *
 * is linked from the top-level documents page.  It can also be found at     *
 * http://hdf.ncsa.uiuc.edu/HDF5/doc/Copyright.html.  If you do not have     *
 * access to either file, you may request a copy from hdfhelp@ncsa.uiuc.edu. *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/* $Id$ */

/*
 * Test parallel HDF5 basic components
 */

#include "testphdf5.h"


/*-------------------------------------------------------------------------
 * Function:    H5FD_mpio_clot_comm_info_deletee
 *
 * Purpose:     Test if communicator and INFO object can be safely deleted
 *		after calling H5Pset_facl_mpio.
 *
 * Return:      Success:        None
 *
 *              Failure:        Abort
 *
 * Programmer:  Albert Cheng
 *              January 9, 2003
 *
 * Modifications:
 *-------------------------------------------------------------------------
 */
void
test_comm_info_delete()
{
    int mpi_size, mpi_rank;
    MPI_Comm comm, comm_tmp;
    int mpi_size_old, mpi_rank_old;
    int mpi_size_tmp, mpi_rank_tmp;
    MPI_Info info = MPI_INFO_NULL;
    MPI_Info info_tmp = MPI_INFO_NULL;
    int mrc;			/* MPI return value */
    hid_t fid;			/* file IDs */
    hid_t acc_pl;		/* File access properties */
    herr_t ret;			/* hdf5 return value */
    int nkeys;

    if (verbose)
	printf("Delete communicator and INFO object\n");

    /* set up MPI parameters */
    MPI_Comm_size(MPI_COMM_WORLD,&mpi_size);
    MPI_Comm_rank(MPI_COMM_WORLD,&mpi_rank);

    /* Create a new communicator that has the same processes as MPI_COMM_WORLD.
     * Use MPI_Comm_split because it is simplier than MPI_Comm_create
     */
    mrc = MPI_Comm_split(MPI_COMM_WORLD, 0, 0, &comm);
    VRFY((mrc==MPI_SUCCESS), "MPI_Comm_split");
    MPI_Comm_size(comm,&mpi_size_old);
    MPI_Comm_rank(comm,&mpi_rank_old);
    if (verbose)
	printf("rank/size of comm are %d/%d\n", mpi_rank_old, mpi_size_old);

    /* create a new INFO object with some trivial information. */
    mrc = MPI_Info_create(&info);
    VRFY((mrc==MPI_SUCCESS), "MPI_Info_create");
    if (mrc = MPI_Info_set(info, "hdf_info_name", "XYZ")) {
	VRFY((mrc==MPI_SUCCESS), "MPI_Info_set");
    }
    (MPI_INFO_NULL != info) && MPI_Info_get_nkeys(info, &nkeys);
    if (verbose)
	h5_dump_info_object(info);

    acc_pl = H5Pcreate (H5P_FILE_ACCESS);
    VRFY((acc_pl >= 0), "H5P_FILE_ACCESS");

    ret = H5Pset_fapl_mpio(acc_pl, comm, info);
    VRFY((ret >= 0), "");

    /* Free the created communicator and INFO object. 
     * Check if the access property list is still valid and can return
     * valid communicator and INFO object.
     */
    mrc = MPI_Comm_free(&comm);
    VRFY((mrc==MPI_SUCCESS), "MPI_Comm_free");
    if (MPI_INFO_NULL!=info){
	mrc = MPI_Info_free(&info);
	VRFY((mrc==MPI_SUCCESS), "MPI_Info_free");
    }

    ret = H5Pget_fapl_mpio(acc_pl, &comm_tmp, &info_tmp);
    VRFY((ret >= 0), "H5Pget_fapl_mpio");
    MPI_Comm_size(comm_tmp,&mpi_size_tmp);
    MPI_Comm_rank(comm_tmp,&mpi_rank_tmp);
    if (verbose)
	printf("After H5Pget_fapl_mpio: rank/size of comm are %d/%d\n",
	mpi_rank_tmp, mpi_size_tmp);
    (MPI_INFO_NULL != info_tmp) && MPI_Info_get_nkeys(info_tmp, &nkeys);
    if (verbose)
	h5_dump_info_object(info_tmp);

    /* Free the retrieved communicator and INFO object.
     * Check if the access property list is still valid and can return
     * valid communicator and INFO object.
     */
    mrc = MPI_Comm_free(&comm_tmp);
    VRFY((mrc==MPI_SUCCESS), "MPI_Comm_free");
    if (MPI_INFO_NULL!=info_tmp){
	mrc = MPI_Info_free(&info_tmp);
	VRFY((mrc==MPI_SUCCESS), "MPI_Info_free");
    }

    ret = H5Pget_fapl_mpio(acc_pl, &comm_tmp, &info_tmp);
    VRFY((ret >= 0), "H5Pget_fapl_mpio");
    MPI_Comm_size(comm_tmp,&mpi_size_tmp);
    MPI_Comm_rank(comm_tmp,&mpi_rank_tmp);
    if (verbose)
	printf("After second H5Pget_fapl_mpio: rank/size of comm are %d/%d\n",
	mpi_rank_tmp, mpi_size_tmp);
    (MPI_INFO_NULL != info_tmp) && MPI_Info_get_nkeys(info_tmp, &nkeys);
    if (verbose)
	h5_dump_info_object(info_tmp);

    /* close the property list and verify the retrieved communicator and INFO
     * object is still valid.
     */
    H5Pclose(acc_pl);
    MPI_Comm_size(comm_tmp,&mpi_size_tmp);
    MPI_Comm_rank(comm_tmp,&mpi_rank_tmp);
    if (verbose)
	printf("After Property list closed: rank/size of comm are %d/%d\n",
	mpi_rank_tmp, mpi_size_tmp);
    (MPI_INFO_NULL != info_tmp) && MPI_Info_get_nkeys(info_tmp, &nkeys);
    if (verbose)
	h5_dump_info_object(info_tmp);

    /* clean up */
    mrc = MPI_Comm_free(&comm_tmp);
    VRFY((mrc==MPI_SUCCESS), "MPI_Comm_free");
    if (MPI_INFO_NULL!=info_tmp){
	mrc = MPI_Info_free(&info_tmp);
	VRFY((mrc==MPI_SUCCESS), "MPI_Info_free");
    }
}
