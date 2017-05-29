/**************************************************************************************************
*                                                                                                 *
* This file is part of HPIPM.                                                                     *
*                                                                                                 *
* HPIPM -- High Performance Interior Point Method.                                                *
* Copyright (C) 2017 by Gianluca Frison.                                                          *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* HPMPC is free software; you can redistribute it and/or                                          *
* modify it under the terms of the GNU Lesser General Public                                      *
* License as published by the Free Software Foundation; either                                    *
* version 2.1 of the License, or (at your option) any later version.                              *
*                                                                                                 *
* HPMPC is distributed in the hope that it will be useful,                                        *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                                            *
* See the GNU Lesser General Public License for more details.                                     *
*                                                                                                 *
* You should have received a copy of the GNU Lesser General Public                                *
* License along with HPMPC; if not, write to the Free Software                                    *
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA                  *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/



#include <blasfeo_target.h>
#include <blasfeo_common.h>
#include <blasfeo_d_aux.h>
#include <blasfeo_d_blas.h>

#include "../include/hpipm_d_dense_qp.h"
#include "../include/hpipm_d_ipm2_hard_dense_qp.h"



void d_compute_res_dense_qp(struct d_dense_qp *qp, struct d_ipm2_hard_dense_qp_workspace *workspace)
	{

	int nv = qp->nv;
	int ne = qp->ne;
	int nb = qp->nb;
	int ng = qp->ng;

	double mu;

	// res g
	dsymv_l_libstr(nv, nv, 1.0, qp->H, 0, 0, workspace->v, 0, 1.0, qp->g, 0, workspace->res_g, 0);

	if(nb>0)
		{
		// res_g
		daxpy_libstr(nb, -1.0, workspace->lam_lb, 0, workspace->lam_ub, 0, workspace->tmp_nb, 0);
		dvecad_sp_libstr(nb, 1.0, workspace->tmp_nb, 0, qp->idxb, workspace->res_g, 0);
		// res_d
		dvecex_sp_libstr(nb, -1.0, qp->idxb, workspace->v, 0, workspace->res_d_lb, 0);
		dveccp_libstr(nb, workspace->res_d_lb, 0, workspace->res_d_ub, 0);
		daxpy_libstr(nb, 1.0, qp->d_lb, 0, workspace->res_d_lb, 0, workspace->res_d_lb, 0);
		daxpy_libstr(nb, 1.0, qp->d_ub, 0, workspace->res_d_ub, 0, workspace->res_d_ub, 0);
		daxpy_libstr(nb, 1.0, workspace->t_lb, 0, workspace->res_d_lb, 0, workspace->res_d_lb, 0);
		daxpy_libstr(nb, -1.0, workspace->t_ub, 0, workspace->res_d_ub, 0, workspace->res_d_ub, 0);
		}

	if(ng>0)
		{
		// TODO
		}
	
	// res b, res g
	dgemv_nt_libstr(ne, nv, -1.0, -1.0, qp->A, 0, 0, workspace->v, 0, workspace->pi, 0, 1.0, 1.0, qp->b, 0, workspace->res_g, 0, workspace->res_b, 0, workspace->res_g, 0);

	// res_mu
	mu = dvecmuldot_libstr(2*nb+2*ng, workspace->lam, 0, workspace->t, 0, workspace->res_m, 0);

	workspace->res_mu = mu*workspace->nt_inv;


	return;

	}
