/**************************************************************************************************
*                                                                                                 *
* This file is part of HPIPM.                                                                     *
*                                                                                                 *
* HPIPM -- High-Performance Interior Point Method.                                                *
* Copyright (C) 2017-2018 by Gianluca Frison.                                                     *
* Developed at IMTEK (University of Freiburg) under the supervision of Moritz Diehl.              *
* All rights reserved.                                                                            *
*                                                                                                 *
* This program is free software: you can redistribute it and/or modify                            *
* it under the terms of the GNU General Public License as published by                            *
* the Free Software Foundation, either version 3 of the License, or                               *
* (at your option) any later version                                                              *.
*                                                                                                 *
* This program is distributed in the hope that it will be useful,                                 *
* but WITHOUT ANY WARRANTY; without even the implied warranty of                                  *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                                   *
* GNU General Public License for more details.                                                    *
*                                                                                                 *
* You should have received a copy of the GNU General Public License                               *
* along with this program.  If not, see <https://www.gnu.org/licenses/>.                          *
*                                                                                                 *
* The authors designate this particular file as subject to the "Classpath" exception              *
* as provided by the authors in the LICENSE file that accompained this code.                      *
*                                                                                                 *
* Author: Gianluca Frison, gianluca.frison (at) imtek.uni-freiburg.de                             *
*                                                                                                 *
**************************************************************************************************/



int SIZEOF_OCP_QP()
	{
	return sizeof(struct OCP_QP);
	}



int MEMSIZE_OCP_QP(struct OCP_QP_DIM *dim)
	{

	// extract dim
	int N = dim->N;
	int *nx = dim->nx;
	int *nu = dim->nu;
	int *nb = dim->nb;
	int *ng = dim->ng;
	int *ns = dim->ns;

	// loop index
	int ii;

	// compute core qp size
	int nvt = 0;
	int net = 0;
	int nct = 0;
	for(ii=0; ii<N; ii++)
		{
		nvt += nx[ii]+nu[ii]+2*ns[ii];
		net += nx[ii+1];
		nct += 2*nb[ii]+2*ng[ii]+2*ns[ii];
		}
	nvt += nx[ii]+nu[ii]+2*ns[ii];
	nct += 2*nb[ii]+2*ng[ii]+2*ns[ii];

	int size = 0;

	size += 5*(N+1)*sizeof(int); // nx nu nb ng ns
	size += 2*(N+1)*sizeof(int *); // idxb idxs
	size += 2*(N+1)*sizeof(struct STRMAT); // RSqrq DCt
	size += 1*N*sizeof(struct STRMAT); // BAbt
	size += 4*(N+1)*sizeof(struct STRVEC); // rqz d m Z
	size += 1*N*sizeof(struct STRVEC); // b

	for(ii=0; ii<N; ii++)
		{
		size += nb[ii]*sizeof(int); // idxb
		size += ns[ii]*sizeof(int); // idxs
		size += SIZE_STRMAT(nu[ii]+nx[ii]+1, nx[ii+1]); // BAbt
		size += SIZE_STRMAT(nu[ii]+nx[ii]+1, nu[ii]+nx[ii]); // RSQrq
		size += SIZE_STRMAT(nu[ii]+nx[ii], ng[ii]); // DCt
		size += SIZE_STRVEC(2*ns[ii]); // Z
		}
	ii = N;
	size += nb[ii]*sizeof(int); // idxb
	size += ns[ii]*sizeof(int); // idxs
	size += SIZE_STRMAT(nu[ii]+nx[ii]+1, nu[ii]+nx[ii]); // RSQrq
	size += SIZE_STRMAT(nu[ii]+nx[ii], ng[ii]); // DCt
	size += SIZE_STRVEC(2*ns[ii]); // Z

	size += 1*SIZE_STRVEC(nvt); // rqz
	size += 1*SIZE_STRVEC(net); // b
	size += 2*SIZE_STRVEC(nct); // d m

	size = (size+63)/64*64; // make multiple of typical cache line size
	size += 64; // align to typical cache line size

	return size;

	}



void CREATE_OCP_QP(struct OCP_QP_DIM *dim, struct OCP_QP *qp, void *mem)
	{

	// extract dim
	int N = dim->N;
	int *nx = dim->nx;
	int *nu = dim->nu;
	int *nb = dim->nb;
	int *ng = dim->ng;
	int *ns = dim->ns;

	// loop index
	int ii;

	// compute core qp size
	int nvt = 0;
	int net = 0;
	int nct = 0;
	for(ii=0; ii<N; ii++)
		{
		nvt += nx[ii]+nu[ii]+2*ns[ii];
		net += nx[ii+1];
		nct += 2*nb[ii]+2*ng[ii]+2*ns[ii];
		}
	nvt += nx[ii]+nu[ii]+2*ns[ii];
	nct += 2*nb[ii]+2*ng[ii]+2*ns[ii];



	// int pointer stuff
	int **ip_ptr;
	ip_ptr = (int **) mem;

	// idxb
	qp->idxb = ip_ptr;
	ip_ptr += N+1;

	// idxs
	qp->idxs = ip_ptr;
	ip_ptr += N+1;


	// matrix struct stuff
	struct STRMAT *sm_ptr = (struct STRMAT *) ip_ptr;

	// BAbt
	qp->BAbt = sm_ptr;
	sm_ptr += N;

	// RSQrq
	qp->RSQrq = sm_ptr;
	sm_ptr += N+1;

	// DCt
	qp->DCt = sm_ptr;
	sm_ptr += N+1;


	// vector struct stuff
	struct STRVEC *sv_ptr = (struct STRVEC *) sm_ptr;

	// b
	qp->b = sv_ptr;
	sv_ptr += N;

	// rqz
	qp->rqz = sv_ptr;
	sv_ptr += N+1;

	// d
	qp->d = sv_ptr;
	sv_ptr += N+1;

	// m
	qp->m = sv_ptr;
	sv_ptr += N+1;

	// Z
	qp->Z = sv_ptr;
	sv_ptr += N+1;


	// integer stuff
	int *i_ptr;
	i_ptr = (int *) sv_ptr;

	// idxb
	for(ii=0; ii<=N; ii++)
		{
		(qp->idxb)[ii] = i_ptr;
		i_ptr += nb[ii];
		}

	// idxs
	for(ii=0; ii<=N; ii++)
		{
		(qp->idxs)[ii] = i_ptr;
		i_ptr += ns[ii];
		}


	// align to typical cache line size
	long long l_ptr = (long long) i_ptr;
	l_ptr = (l_ptr+63)/64*64;


	// floating point stuff
	char *c_ptr;
	c_ptr = (char *) l_ptr;

	char *tmp_ptr;

	// BAbt
	for(ii=0; ii<N; ii++)
		{
		CREATE_STRMAT(nu[ii]+nx[ii]+1, nx[ii+1], qp->BAbt+ii, c_ptr);
		c_ptr += (qp->BAbt+ii)->memsize;
		GESE(nu[ii]+nx[ii]+1, nx[ii+1], 0.0, qp->BAbt+ii, 0, 0);
		}

	// RSQrq
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRMAT(nu[ii]+nx[ii]+1, nu[ii]+nx[ii], qp->RSQrq+ii, c_ptr);
		c_ptr += (qp->RSQrq+ii)->memsize;
		GESE(nu[ii]+nx[ii]+1, nu[ii]+nx[ii], 0.0, qp->RSQrq+ii, 0, 0);
		}

	// DCt
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRMAT(nu[ii]+nx[ii], ng[ii], qp->DCt+ii, c_ptr);
		c_ptr += (qp->DCt+ii)->memsize;
		GESE(nu[ii]+nx[ii], ng[ii], 0.0, qp->DCt+ii, 0, 0);
		}

	// Z
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*ns[ii], qp->Z+ii, c_ptr);
		c_ptr += (qp->Z+ii)->memsize;
		VECSE(2*ns[ii], 0.0, qp->Z+ii, 0);
		}

	// g
	tmp_ptr = c_ptr;
	c_ptr += SIZE_STRVEC(nvt);
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(nu[ii]+nx[ii]+2*ns[ii], qp->rqz+ii, tmp_ptr);
		tmp_ptr += nu[ii]*sizeof(REAL);
		tmp_ptr += nx[ii]*sizeof(REAL);
		tmp_ptr += ns[ii]*sizeof(REAL);
		tmp_ptr += ns[ii]*sizeof(REAL);
		VECSE(nu[ii]+nx[ii]+2*ns[ii], 0.0, qp->rqz+ii, 0);
		}

	// b
	tmp_ptr = c_ptr;
	c_ptr += SIZE_STRVEC(net);
	for(ii=0; ii<N; ii++)
		{
		CREATE_STRVEC(nx[ii+1], qp->b+ii, tmp_ptr);
		tmp_ptr += nx[ii+1]*sizeof(REAL);
		VECSE(nx[ii+1], 0.0, qp->b+ii, 0);
		}

	// d
	tmp_ptr = c_ptr;
	c_ptr += SIZE_STRVEC(nct);
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*nb[ii]+2*ng[ii]+2*ns[ii], qp->d+ii, tmp_ptr);
		tmp_ptr += nb[ii]*sizeof(REAL); // lb
		tmp_ptr += ng[ii]*sizeof(REAL); // lg
		tmp_ptr += nb[ii]*sizeof(REAL); // ub
		tmp_ptr += ng[ii]*sizeof(REAL); // ug
		tmp_ptr += ns[ii]*sizeof(REAL); // ls
		tmp_ptr += ns[ii]*sizeof(REAL); // us
		VECSE(2*nb[ii]+2*ng[ii]+2*ns[ii], 0.0, qp->d+ii, 0);
		}

	// m
	tmp_ptr = c_ptr;
	c_ptr += SIZE_STRVEC(nct);
	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*nb[ii]+2*ng[ii]+2*ns[ii], qp->m+ii, tmp_ptr);
		tmp_ptr += nb[ii]*sizeof(REAL); // lb
		tmp_ptr += ng[ii]*sizeof(REAL); // lg
		tmp_ptr += nb[ii]*sizeof(REAL); // ub
		tmp_ptr += ng[ii]*sizeof(REAL); // ug
		tmp_ptr += ns[ii]*sizeof(REAL); // ls
		tmp_ptr += ns[ii]*sizeof(REAL); // us
		VECSE(2*nb[ii]+2*ng[ii]+2*ns[ii], 0.0, qp->m+ii, 0);
		}

	qp->dim = dim;

	qp->memsize = MEMSIZE_OCP_QP(dim);


#if defined(RUNTIME_CHECKS)
	if(c_ptr > ((char *) mem) + qp->memsize)
		{
		printf("\nCreate_ocp_qp: outside memory bounds!\n\n");
		exit(1);
		}
#endif


	return;

	}



void CVT_COLMAJ_TO_OCP_QP(REAL **A, REAL **B, REAL **b, REAL **Q, REAL **S, REAL **R, REAL **q, REAL **r, int **idxb, REAL **d_lb, REAL **d_ub, REAL **C, REAL **D, REAL **d_lg, REAL **d_ug, REAL **Zl, REAL **Zu, REAL **zl, REAL **zu, int **idxs, REAL **d_ls, REAL **d_us, struct OCP_QP *qp)
	{

	// extract dim
	int N = qp->dim->N;
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;
	int *ns = qp->dim->ns;

	int ii, jj;

	for(ii=0; ii<N; ii++)
		{
		CVT_TRAN_MAT2STRMAT(nx[ii+1], nu[ii], B[ii], nx[ii+1], qp->BAbt+ii, 0, 0);
		CVT_TRAN_MAT2STRMAT(nx[ii+1], nx[ii], A[ii], nx[ii+1], qp->BAbt+ii, nu[ii], 0);
		CVT_TRAN_MAT2STRMAT(nx[ii+1], 1, b[ii], nx[ii+1], qp->BAbt+ii, nu[ii]+nx[ii], 0); // XXX remove ???
		CVT_VEC2STRVEC(nx[ii+1], b[ii], qp->b+ii, 0);
		}

	for(ii=0; ii<=N; ii++)
		{
		CVT_MAT2STRMAT(nu[ii], nu[ii], R[ii], nu[ii], qp->RSQrq+ii, 0, 0);
		CVT_TRAN_MAT2STRMAT(nu[ii], nx[ii], S[ii], nu[ii], qp->RSQrq+ii, nu[ii], 0);
		CVT_MAT2STRMAT(nx[ii], nx[ii], Q[ii], nx[ii], qp->RSQrq+ii, nu[ii], nu[ii]);
		CVT_TRAN_MAT2STRMAT(nu[ii], 1, r[ii], nu[ii], qp->RSQrq+ii, nu[ii]+nx[ii], 0); // XXX remove ???
		CVT_TRAN_MAT2STRMAT(nx[ii], 1, q[ii], nx[ii], qp->RSQrq+ii, nu[ii]+nx[ii], nu[ii]); // XXX remove ???
		CVT_VEC2STRVEC(nu[ii], r[ii], qp->rqz+ii, 0);
		CVT_VEC2STRVEC(nx[ii], q[ii], qp->rqz+ii, nu[ii]);
		}

	for(ii=0; ii<=N; ii++)
		{
		if(nb[ii]>0)
			{
			for(jj=0; jj<nb[ii]; jj++)
				qp->idxb[ii][jj] = idxb[ii][jj];
			CVT_VEC2STRVEC(nb[ii], d_lb[ii], qp->d+ii, 0);
			CVT_VEC2STRVEC(nb[ii], d_ub[ii], qp->d+ii, nb[ii]+ng[ii]);
			VECSC_LIBSTR(nb[ii], -1.0, qp->d+ii, nb[ii]+ng[ii]);
			VECSE(nb[ii], 0.0, qp->m+ii, 0);
			VECSE(nb[ii], 0.0, qp->m+ii, nb[ii]+ng[ii]);
			}
		}

	for(ii=0; ii<=N; ii++)
		{
		if(ng[ii]>0)
			{
			CVT_TRAN_MAT2STRMAT(ng[ii], nu[ii], D[ii], ng[ii], qp->DCt+ii, 0, 0);
			CVT_TRAN_MAT2STRMAT(ng[ii], nx[ii], C[ii], ng[ii], qp->DCt+ii, nu[ii], 0);
			CVT_VEC2STRVEC(ng[ii], d_lg[ii], qp->d+ii, nb[ii]);
			CVT_VEC2STRVEC(ng[ii], d_ug[ii], qp->d+ii, 2*nb[ii]+ng[ii]);
			VECSC_LIBSTR(ng[ii], -1.0, qp->d+ii, 2*nb[ii]+ng[ii]);
			VECSE(ng[ii], 0.0, qp->m+ii, nb[ii]);
			VECSE(ng[ii], 0.0, qp->m+ii, 2*nb[ii]+ng[ii]);
			}
		}

	for(ii=0; ii<=N; ii++)
		{
		if(ns[ii]>0)
			{
			for(jj=0; jj<ns[ii]; jj++)
				qp->idxs[ii][jj] = idxs[ii][jj];
			CVT_VEC2STRVEC(ns[ii], Zl[ii], qp->Z+ii, 0);
			CVT_VEC2STRVEC(ns[ii], Zu[ii], qp->Z+ii, ns[ii]);
			CVT_VEC2STRVEC(ns[ii], zl[ii], qp->rqz+ii, nu[ii]+nx[ii]);
			CVT_VEC2STRVEC(ns[ii], zu[ii], qp->rqz+ii, nu[ii]+nx[ii]+ns[ii]);
			CVT_VEC2STRVEC(ns[ii], d_ls[ii], qp->d+ii, 2*nb[ii]+2*ng[ii]);
			CVT_VEC2STRVEC(ns[ii], d_us[ii], qp->d+ii, 2*nb[ii]+2*ng[ii]+ns[ii]);
			VECSE(ns[ii], 0.0, qp->m+ii, 2*nb[ii]+2*ng[ii]);
			VECSE(ns[ii], 0.0, qp->m+ii, 2*nb[ii]+2*ng[ii]+ns[ii]);
			}
		}

	return;

	}



void CVT_ROWMAJ_TO_OCP_QP(REAL **A, REAL **B, REAL **b, REAL **Q, REAL **S, REAL **R, REAL **q, REAL **r, int **idxb, REAL **d_lb, REAL **d_ub, REAL **C, REAL **D, REAL **d_lg, REAL **d_ug, REAL **Zl, REAL **Zu, REAL **zl, REAL **zu, int **idxs, REAL **d_ls, REAL **d_us, struct OCP_QP *qp)
	{

	// extract dim
	int N = qp->dim->N;
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;
	int *ns = qp->dim->ns;

	int ii, jj;

	for(ii=0; ii<N; ii++)
		{
		CVT_MAT2STRMAT(nu[ii], nx[ii+1], B[ii], nu[ii], qp->BAbt+ii, 0, 0);
		CVT_MAT2STRMAT(nx[ii], nx[ii+1], A[ii], nx[ii], qp->BAbt+ii, nu[ii], 0);
		CVT_TRAN_MAT2STRMAT(nx[ii+1], 1, b[ii], nx[ii+1], qp->BAbt+ii, nu[ii]+nx[ii], 0);
		CVT_VEC2STRVEC(nx[ii+1], b[ii], qp->b+ii, 0);
		}

	for(ii=0; ii<=N; ii++)
		{
		CVT_TRAN_MAT2STRMAT(nu[ii], nu[ii], R[ii], nu[ii], qp->RSQrq+ii, 0, 0);
		CVT_MAT2STRMAT(nx[ii], nu[ii], S[ii], nx[ii], qp->RSQrq+ii, nu[ii], 0);
		CVT_TRAN_MAT2STRMAT(nx[ii], nx[ii], Q[ii], nx[ii], qp->RSQrq+ii, nu[ii], nu[ii]);
		CVT_TRAN_MAT2STRMAT(nu[ii], 1, r[ii], nu[ii], qp->RSQrq+ii, nu[ii]+nx[ii], 0);
		CVT_TRAN_MAT2STRMAT(nx[ii], 1, q[ii], nx[ii], qp->RSQrq+ii, nu[ii]+nx[ii], nu[ii]);
		CVT_VEC2STRVEC(nu[ii], r[ii], qp->rqz+ii, 0);
		CVT_VEC2STRVEC(nx[ii], q[ii], qp->rqz+ii, nu[ii]);
		}

	for(ii=0; ii<=N; ii++)
		{
		if(nb[ii]>0)
			{
			for(jj=0; jj<nb[ii]; jj++)
				qp->idxb[ii][jj] = idxb[ii][jj];
			CVT_VEC2STRVEC(nb[ii], d_lb[ii], qp->d+ii, 0);
			CVT_VEC2STRVEC(nb[ii], d_ub[ii], qp->d+ii, nb[ii]+ng[ii]);
			VECSC_LIBSTR(nb[ii], -1.0, qp->d+ii, nb[ii]+ng[ii]);
			VECSE(nb[ii], 0.0, qp->m+ii, 0);
			VECSE(nb[ii], 0.0, qp->m+ii, nb[ii]+ng[ii]);
			}
		}

	for(ii=0; ii<=N; ii++)
		{
		if(ng[ii]>0)
			{
			CVT_MAT2STRMAT(nu[ii], ng[ii], D[ii], nu[ii], qp->DCt+ii, 0, 0);
			CVT_MAT2STRMAT(nx[ii], ng[ii], C[ii], nx[ii], qp->DCt+ii, nu[ii], 0);
			CVT_VEC2STRVEC(ng[ii], d_lg[ii], qp->d+ii, nb[ii]);
			CVT_VEC2STRVEC(ng[ii], d_ug[ii], qp->d+ii, 2*nb[ii]+ng[ii]);
			VECSC_LIBSTR(ng[ii], -1.0, qp->d+ii, 2*nb[ii]+ng[ii]);
			VECSE(ng[ii], 0.0, qp->m+ii, nb[ii]);
			VECSE(ng[ii], 0.0, qp->m+ii, 2*nb[ii]+ng[ii]);
			}
		}

	for(ii=0; ii<=N; ii++)
		{
		if(ns[ii]>0)
			{
			for(jj=0; jj<ns[ii]; jj++)
				qp->idxs[ii][jj] = idxs[ii][jj];
			CVT_VEC2STRVEC(ns[ii], Zl[ii], qp->Z+ii, 0);
			CVT_VEC2STRVEC(ns[ii], Zu[ii], qp->Z+ii, ns[ii]);
			CVT_VEC2STRVEC(ns[ii], zl[ii], qp->rqz+ii, nu[ii]+nx[ii]);
			CVT_VEC2STRVEC(ns[ii], zu[ii], qp->rqz+ii, nu[ii]+nx[ii]+ns[ii]);
			CVT_VEC2STRVEC(ns[ii], d_ls[ii], qp->d+ii, 2*nb[ii]+2*ng[ii]);
			CVT_VEC2STRVEC(ns[ii], d_us[ii], qp->d+ii, 2*nb[ii]+2*ng[ii]+ns[ii]);
			VECSE(ns[ii], 0.0, qp->m+ii, 2*nb[ii]+2*ng[ii]);
			VECSE(ns[ii], 0.0, qp->m+ii, 2*nb[ii]+2*ng[ii]+ns[ii]);
			}
		}

	return;

	}


// convert generic field
void CVT_COLMAJ_TO_OCP_QP_GF(char *field_name, int stage, REAL *in, struct OCP_QP *qp)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;
	int *ns = qp->dim->ns;
    
	// matrices
	if(hpipm_strcmp(field_name, "A")) 
		{
		CVT_TRAN_MAT2STRMAT(nx[stage+1], nx[stage], in, nx[stage+1], qp->BAbt+stage, nu[stage], 0);
		}
	else if(hpipm_strcmp(field_name, "B")) 
		{
		CVT_TRAN_MAT2STRMAT(nx[stage+1], nu[stage], in, nx[stage+1], qp->BAbt+stage, 0, 0);
		}
	else if(hpipm_strcmp(field_name, "Q")) 
		{
		CVT_MAT2STRMAT(nx[stage], nx[stage], in, nx[stage], qp->RSQrq+stage, nu[stage], nu[stage]);
		}
	else if(hpipm_strcmp(field_name, "S")) 
		{
		CVT_TRAN_MAT2STRMAT(nu[stage], nx[stage], in, nu[stage], qp->RSQrq+stage, nu[stage], 0);
		}
	else if(hpipm_strcmp(field_name, "R")) 
		{
		CVT_MAT2STRMAT(nu[stage], nu[stage], in, nu[stage], qp->RSQrq+stage, 0, 0);
		}
	else if(hpipm_strcmp(field_name, "C")) 
		{
		CVT_TRAN_MAT2STRMAT(ng[stage], nx[stage], in, ng[stage], qp->DCt+stage, nu[stage], 0);
		}
	else if(hpipm_strcmp(field_name, "D")) 
		{
		CVT_TRAN_MAT2STRMAT(ng[stage], nu[stage], in, ng[stage], qp->DCt+stage, 0, 0);
		}
	// vectors
	else if(hpipm_strcmp(field_name, "b") == 1)
		{ 
		CVT_TRAN_MAT2STRMAT(nx[stage+1], 1, in, nx[stage+1], &(qp->BAbt[stage]), nx[stage]+nu[stage], 0);
		CVT_VEC2STRVEC(nx[stage+1], in, qp->b+stage, 0);
		}
	else if(hpipm_strcmp(field_name, "q"))
		{ 
		CVT_TRAN_MAT2STRMAT(nx[stage], 1, in, nx[stage], &(qp->RSQrq[stage]), nx[stage]+nu[stage], nu[stage]);
		CVT_VEC2STRVEC(nx[stage], in, qp->rqz+stage, nu[stage]);
		}
	else if(hpipm_strcmp(field_name, "r"))
		{ 
		CVT_TRAN_MAT2STRMAT(nu[stage], 1, in, nu[stage], &(qp->RSQrq[stage]), nx[stage]+nu[stage], 0);
		CVT_VEC2STRVEC(nu[stage], in, qp->rqz+stage, 0);
		}
	else if(hpipm_strcmp(field_name, "lb"))
		{ 
		CVT_VEC2STRVEC(nb[stage], in, qp->d+stage, 0);
		}
	else if(hpipm_strcmp(field_name, "ub"))
		{ 
		CVT_VEC2STRVEC(nb[stage], in, qp->d+stage, nb[stage]+ng[stage]);
		VECSC_LIBSTR(nb[stage], -1.0, qp->d+stage, nb[stage]+ng[stage]);
		}
	else if(hpipm_strcmp(field_name, "lg"))
		{ 
		CVT_VEC2STRVEC(ng[stage], in, qp->d+stage, nb[stage]);
		}
	else if(hpipm_strcmp(field_name, "ug"))
		{ 
		CVT_VEC2STRVEC(ng[stage], in, qp->d+stage, 2*nb[stage]+ng[stage]);
		VECSC_LIBSTR(ng[stage], -1.0, qp->d+stage, 2*nb[stage]+ng[stage]);
		}
	else if(hpipm_strcmp(field_name, "Zl"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->Z+stage, 0);
		}
	else if(hpipm_strcmp(field_name, "Zu"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->Z+stage, ns[stage]);
		}
	else if(hpipm_strcmp(field_name, "zl"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->rqz+stage, nu[stage]+nx[stage]);
		}
	else if(hpipm_strcmp(field_name, "zu"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->rqz+stage, nu[stage]+nx[stage]+ns[stage]);
		}
	else if(hpipm_strcmp(field_name, "lls"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->d+stage, 2*nb[stage]+2*ng[stage]);
		}
	else if(hpipm_strcmp(field_name, "lus"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->d+stage, 2*nb[stage]+2*ng[stage]+ns[stage]);
		}
	else
		{
		printf("error [CVT_COLMAJ_MAT_TO_OCP_QP]: unknown field name '%s'. Exiting.\n", field_name);
		exit(1);	
		}
	return;
	}



void CVT_COLMAJ_MAT_TO_OCP_QP(char *field_name, int stage, REAL *in, struct OCP_QP *qp)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;
	int *ng = qp->dim->ng;
    
	if(hpipm_strcmp(field_name, "A")) 
		{
		CVT_TRAN_MAT2STRMAT(nx[stage+1], nx[stage], in, nx[stage+1], qp->BAbt+stage, nu[stage], 0);
		}
	else if(hpipm_strcmp(field_name, "B")) 
		{
		CVT_TRAN_MAT2STRMAT(nx[stage+1], nu[stage], in, nx[stage+1], qp->BAbt+stage, 0, 0);
		}
	else if(hpipm_strcmp(field_name, "Q")) 
		{
		CVT_MAT2STRMAT(nx[stage], nx[stage], in, nx[stage], qp->RSQrq+stage, nu[stage], nu[stage]);
		}
	else if(hpipm_strcmp(field_name, "S")) 
		{
		CVT_TRAN_MAT2STRMAT(nu[stage], nx[stage], in, nu[stage], qp->RSQrq+stage, nu[stage], 0);
		}
	else if(hpipm_strcmp(field_name, "R")) 
		{
		CVT_MAT2STRMAT(nu[stage], nu[stage], in, nu[stage], qp->RSQrq+stage, 0, 0);
		}
	else if(hpipm_strcmp(field_name, "C")) 
		{
		CVT_TRAN_MAT2STRMAT(ng[stage], nx[stage], in, ng[stage], qp->DCt+stage, nu[stage], 0);
		}
	else if(hpipm_strcmp(field_name, "D")) 
		{
		CVT_TRAN_MAT2STRMAT(ng[stage], nu[stage], in, ng[stage], qp->DCt+stage, 0, 0);
		}
	else
		{
		printf("error [CVT_COLMAJ_MAT_TO_OCP_QP]: unknown field name '%s'. Exiting.\n", field_name);
		exit(1);	
		}
	return;
	}



void CVT_OCP_QP_TO_COLMAJ_MAT(char *field_name, int stage, struct OCP_QP *qp, REAL *out)
    {
    // extract dim
    int *nx = qp->dim->nx;
    int *nu = qp->dim->nu;
	int *ng = qp->dim->ng;

	if(hpipm_strcmp(field_name, "A")) 
		{
		CVT_TRAN_STRMAT2MAT(nx[stage], nx[stage+1], qp->BAbt+stage, nu[stage], 0, out, nx[stage+1]);
		}
	else if(hpipm_strcmp(field_name, "B")) 
		{
		CVT_TRAN_STRMAT2MAT(nu[stage], nx[stage+1], qp->BAbt+stage, 0, 0, out, nx[stage+1]);
		}
	else if(hpipm_strcmp(field_name, "Q")) 
		{
		CVT_STRMAT2MAT(nx[stage], nx[stage], qp->RSQrq+stage, nu[stage], nu[stage], out, nx[stage]);
		}
	else if(hpipm_strcmp(field_name, "S")) 
		{
		CVT_TRAN_STRMAT2MAT(nx[stage], nu[stage], qp->RSQrq+stage, nu[stage], 0, out, nu[stage]);
		}
	else if(hpipm_strcmp(field_name, "R")) 
		{
		CVT_STRMAT2MAT(nu[stage], nu[stage], qp->RSQrq+stage, 0, 0, out, nu[stage]);
		}
	else if(hpipm_strcmp(field_name, "C")) 
		{
		CVT_TRAN_STRMAT2MAT(nx[stage], ng[stage], qp->DCt+stage, nu[stage], 0, out, ng[stage]);
		}
	else if(hpipm_strcmp(field_name, "D")) 
		{
		CVT_TRAN_STRMAT2MAT(nu[stage], ng[stage], qp->DCt+stage, 0, 0, out, ng[stage]);
		}
	else
		{
		printf("error [CVT_OCP_QP_TO_COLMAJ_MAT]: unknown field name '%s'. Exiting.\n", field_name);
		exit(1);	
		}
	return;
	}



void CVT_COLMAJ_VEC_TO_OCP_QP(char *field_name, int stage, REAL *in, struct OCP_QP *qp)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;
	int *ns = qp->dim->ns;
    
	if(hpipm_strcmp(field_name, "b") == 1)
		{ 
		CVT_TRAN_MAT2STRMAT(nx[stage+1], 1, in, nx[stage+1], &(qp->BAbt[stage]), nx[stage]+nu[stage], 0);
		CVT_VEC2STRVEC(nx[stage+1], in, qp->b+stage, 0);
		}
	else if(hpipm_strcmp(field_name, "q"))
		{ 
		CVT_TRAN_MAT2STRMAT(nx[stage], 1, in, nx[stage], &(qp->RSQrq[stage]), nx[stage]+nu[stage], nu[stage]);
		CVT_VEC2STRVEC(nx[stage], in, qp->rqz+stage, nu[stage]);
		}
	else if(hpipm_strcmp(field_name, "r"))
		{ 
		CVT_TRAN_MAT2STRMAT(nu[stage], 1, in, nu[stage], &(qp->RSQrq[stage]), nx[stage]+nu[stage], 0);
		CVT_VEC2STRVEC(nu[stage], in, qp->rqz+stage, 0);
		}
	else if(hpipm_strcmp(field_name, "lb"))
		{ 
		CVT_VEC2STRVEC(nb[stage], in, qp->d+stage, 0);
		}
	else if(hpipm_strcmp(field_name, "ub"))
		{ 
		CVT_VEC2STRVEC(nb[stage], in, qp->d+stage, nb[stage]+ng[stage]);
		VECSC_LIBSTR(nb[stage], -1.0, qp->d+stage, nb[stage]+ng[stage]);
		}
	else if(hpipm_strcmp(field_name, "lg"))
		{ 
		CVT_VEC2STRVEC(ng[stage], in, qp->d+stage, nb[stage]);
		}
	else if(hpipm_strcmp(field_name, "ug"))
		{ 
		CVT_VEC2STRVEC(ng[stage], in, qp->d+stage, 2*nb[stage]+ng[stage]);
		VECSC_LIBSTR(ng[stage], -1.0, qp->d+stage, 2*nb[stage]+ng[stage]);
		}
	else if(hpipm_strcmp(field_name, "Zl"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->Z+stage, 0);
		}
	else if(hpipm_strcmp(field_name, "Zu"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->Z+stage, ns[stage]);
		}
	else if(hpipm_strcmp(field_name, "zl"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->rqz+stage, nu[stage]+nx[stage]);
		}
	else if(hpipm_strcmp(field_name, "zu"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->rqz+stage, nu[stage]+nx[stage]+ns[stage]);
		}
	else if(hpipm_strcmp(field_name, "lls"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->d+stage, 2*nb[stage]+2*ng[stage]);
		}
	else if(hpipm_strcmp(field_name, "lus"))
		{ 
		CVT_VEC2STRVEC(ns[stage], in, qp->d+stage, 2*nb[stage]+2*ng[stage]+ns[stage]);
		}
	else
		{
		printf("error [CVT_COLMAJ_VEC_TO_OCP_QP]: unknown field name '%s'. Exiting.\n", field_name);
		printf("%c %c", field_name[0], field_name[1]);
		exit(1);	
		}
	return;
	}



void CVT_OCP_QP_TO_COLMAJ_VEC(char *field_name, int stage, struct OCP_QP *qp, REAL *out)
    {
	int ii;
    // extract dim
    int *nx = qp->dim->nx;
    int *nu = qp->dim->nu;
    int *nb = qp->dim->nb;
    int *ng = qp->dim->ng;
    int *ns = qp->dim->ns;

	if(hpipm_strcmp(field_name, "b") == 1) 
		{
		CVT_STRVEC2VEC(nx[stage+1], qp->b+stage, 0, out);
		}
	if(hpipm_strcmp(field_name, "q") == 1) 
		{
		CVT_STRVEC2VEC(nx[stage], qp->rqz+stage, nu[stage], out);
		}
	if(hpipm_strcmp(field_name, "r") == 1) 
		{
		CVT_STRVEC2VEC(nu[stage], qp->rqz+stage, 0, out);
		}
	if(hpipm_strcmp(field_name, "lb") == 1) 
		{
		CVT_STRVEC2VEC(nb[stage], qp->d+stage, 0, out);
		}
	if(hpipm_strcmp(field_name, "ub") == 1) 
		{
		CVT_STRVEC2VEC(nb[stage], qp->d+stage, nb[stage]+ng[stage], out);
		for (ii=0; ii<nb[stage]; ii++) out[ii] = - out[ii];
		}
	if(hpipm_strcmp(field_name, "lg") == 1) 
		{
		CVT_STRVEC2VEC(ng[stage], qp->d+stage, nb[stage], out);
		}
	if(hpipm_strcmp(field_name, "ug") == 1) 
		{
		CVT_STRVEC2VEC(ng[stage], qp->d+stage, 2*nb[stage]+ng[stage], out);
		for (ii=0; ii<ng[stage]; ii++) out[ii] = - out[ii];
		}
	if(hpipm_strcmp(field_name, "Zl") == 1) 
		{
		CVT_STRVEC2VEC(ns[stage], qp->Z+stage, 0, out);
		}
	if(hpipm_strcmp(field_name, "Zu") == 1) 
		{
		CVT_STRVEC2VEC(ns[stage], qp->Z+stage, ns[stage], out);
		}
	if(hpipm_strcmp(field_name, "zl") == 1) 
		{
		CVT_STRVEC2VEC(ns[stage], qp->rqz+stage, nu[stage]+nx[stage], out);
		}
	if(hpipm_strcmp(field_name, "zu") == 1) 
		{
		CVT_STRVEC2VEC(ns[stage], qp->rqz+stage, nu[stage]+nx[stage]+ns[stage], out);
		}
	if(hpipm_strcmp(field_name, "lls") == 1) 
		{
		CVT_STRVEC2VEC(ns[stage], qp->d+stage, 2*nb[stage]+2*ng[stage], out);
		}
	if(hpipm_strcmp(field_name, "lus") == 1) 
		{
		CVT_STRVEC2VEC(ns[stage], qp->d+stage, 2*nb[stage]+2*ng[stage]+ns[stage], out);
		}
	else
		{
		printf("error [CVT_OCP_QP_TO_COLMAJ_VEC]: unknown field name '%s'. Exiting.\n", field_name);
		exit(1);
		}
	return;
	}



void CVT_COLMAJ_TO_OCP_QP_A(int stage, REAL *A, struct OCP_QP *qp)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	CVT_TRAN_MAT2STRMAT(nx[stage+1], nx[stage], A, nx[stage+1], qp->BAbt+stage, nu[stage], 0);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_A(int stage, struct OCP_QP *qp, REAL *A)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	CVT_TRAN_STRMAT2MAT(nx[stage], nx[stage+1], qp->BAbt+stage, nu[stage], 0, A, nx[stage+1]);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_B(int stage, REAL *B, struct OCP_QP *qp)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	CVT_TRAN_MAT2STRMAT(nx[stage+1], nu[stage], B, nx[stage+1], qp->BAbt+stage, 0, 0);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_B(int stage, struct OCP_QP *qp, REAL *B)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	CVT_TRAN_STRMAT2MAT(nu[stage], nx[stage+1], qp->BAbt+stage, 0, 0, B, nx[stage+1]);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_BVEC(int stage, REAL *b, struct OCP_QP *qp)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	int row_offset = qp->dim->nx[stage] + qp->dim->nu[stage], col_offset = 0;
	CVT_TRAN_MAT2STRMAT(nx[stage+1], 1, b, nx[stage+1], &(qp->BAbt[stage]), row_offset, col_offset);
	CVT_VEC2STRVEC(nx[stage+1], b, qp->b+stage, 0);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_BVEC(int stage, struct OCP_QP *qp, REAL *b)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	CVT_STRVEC2VEC(nx[stage+1], qp->b+stage, 0, b);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_Q(int stage, REAL *Q, struct OCP_QP *qp)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	CVT_MAT2STRMAT(nx[stage], nx[stage], Q, nx[stage], qp->RSQrq+stage, nu[stage], nu[stage]);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_Q(int stage, struct OCP_QP *qp, REAL *Q)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	CVT_STRMAT2MAT(nx[stage], nx[stage], qp->RSQrq+stage, nu[stage], nu[stage], Q, nx[stage]);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_S(int stage, REAL *S, struct OCP_QP *qp)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	CVT_TRAN_MAT2STRMAT(nu[stage], nx[stage], S, nu[stage], qp->RSQrq+stage, nu[stage], 0);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_S(int stage, struct OCP_QP *qp, REAL *S)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	CVT_TRAN_STRMAT2MAT(nx[stage], nu[stage], qp->RSQrq+stage, nu[stage], 0, S, nu[stage]);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_R(int stage, REAL *R, struct OCP_QP *qp)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	CVT_MAT2STRMAT(nu[stage], nu[stage], R, nu[stage], qp->RSQrq+stage, 0, 0);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_R(int stage, struct OCP_QP *qp, REAL *R)
	{
	// extract dim
	int *nu = qp->dim->nu;

	CVT_STRMAT2MAT(nu[stage], nu[stage], qp->RSQrq+stage, 0, 0, R, nu[stage]);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_QVEC(int stage, REAL *q, struct OCP_QP *qp)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	int row_offset = qp->dim->nu[stage] + qp->dim->nx[stage], col_offset = qp->dim->nu[stage];
 	CVT_TRAN_MAT2STRMAT(nx[stage], 1, q, nx[stage], &(qp->RSQrq[stage]), row_offset, col_offset);
	CVT_VEC2STRVEC(nx[stage], q, qp->rqz+stage, nu[stage]);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_QVEC(int stage, struct OCP_QP *qp, REAL *q)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;

	CVT_STRVEC2VEC(nx[stage], qp->rqz+stage, nu[stage], q);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_RVEC(int stage, REAL *r, struct OCP_QP *qp)
	{
	// extract dim
	int *nu = qp->dim->nu;
	int row_offset = qp->dim->nu[stage] + qp->dim->nx[stage], col_offset = 0;
	CVT_TRAN_MAT2STRMAT(nu[stage], 1, r, nu[stage], &(qp->RSQrq[stage]), row_offset, col_offset);
	CVT_VEC2STRVEC(nu[stage], r, qp->rqz+stage, 0);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_RVEC(int stage, struct OCP_QP *qp, REAL *r)
	{
	// extract dim
	int *nu = qp->dim->nu;

	CVT_STRVEC2VEC(nu[stage], qp->rqz+stage, 0, r);

	return;
	}



// TODO remove !!!
void CVT_COLMAJ_TO_OCP_QP_LBX(int stage, REAL *lbx, struct OCP_QP *qp)
	{
	// extract dim
	int *nbu = qp->dim->nbu;
	int *nbx = qp->dim->nbx;

	CVT_VEC2STRVEC(nbx[stage], lbx, qp->d+stage, nbu[stage]);

	return;
	}



// TODO remove !!!
void CVT_OCP_QP_TO_COLMAJ_LBX(int stage, struct OCP_QP *qp, REAL *lbx)
	{
	// extract dim
	int *nbu = qp->dim->nbu;
	int *nbx = qp->dim->nbx;

	CVT_STRVEC2VEC(nbx[stage], qp->d+stage, nbu[stage], lbx);

	return;
	}



// TODO remove !!!
void CVT_COLMAJ_TO_OCP_QP_LBU(int stage, REAL *lbu, struct OCP_QP *qp)
	{
	// extract dim
	int *nbu = qp->dim->nbu;

	CVT_VEC2STRVEC(nbu[stage], lbu, qp->d+stage, 0);

	return;
	}



// TODO remove !!!
void CVT_OCP_QP_TO_COLMAJ_LBU(int stage, struct OCP_QP *qp, REAL *lbu)
	{
	// extract dim
	int *nbu = qp->dim->nbu;

	CVT_STRVEC2VEC(nbu[stage], qp->d+stage, 0, lbu);

	return;
	}



// TODO remove !!!
void CVT_COLMAJ_TO_OCP_QP_UBX(int stage, REAL *lbx, struct OCP_QP *qp)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *nbx = qp->dim->nbx;
	int *nbu = qp->dim->nbu;
	int *ng = qp->dim->ng;

	CVT_VEC2STRVEC(nbx[stage], lbx, qp->d+stage, nb[stage]+ng[stage]+nbu[stage]);
	VECSC_LIBSTR(nbx[stage], -1.0, qp->d+stage, nb[stage]+ng[stage]+nbu[stage]);

	return;
	}



// TODO remove !!!
void CVT_OCP_QP_TO_COLMAJ_UBX(int stage, struct OCP_QP *qp, REAL *ubx)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *nbx = qp->dim->nbx;
	int *nbu = qp->dim->nbu;
	int *ng = qp->dim->ng;

	int i;

	CVT_STRVEC2VEC(nbx[stage], qp->d+stage, nb[stage]+ng[stage]+nbu[stage], ubx);
	for(i=0; i<nbx[stage]; i++)
		{
		ubx[i] = -ubx[i];
		}

	return;
	}



// TODO remove !!!
void CVT_COLMAJ_TO_OCP_QP_UBU(int stage, REAL *ubu, struct OCP_QP *qp)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *nbu = qp->dim->nbu;
	int *ng = qp->dim->ng;

	CVT_VEC2STRVEC(nbu[stage], ubu, qp->d+stage, nb[stage]+ng[stage]);
	VECSC_LIBSTR(nbu[stage], -1.0, qp->d+stage, nb[stage]+ng[stage]);

	return;
	}



// TODO remove !!!
void CVT_OCP_QP_TO_COLMAJ_UBU(int stage, struct OCP_QP *qp, REAL *ubu)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *nbu = qp->dim->nbu;
	int *ng = qp->dim->ng;

	int i;

	CVT_STRVEC2VEC(nbu[stage], qp->d+stage, nb[stage]+ng[stage], ubu);
	for(i=0; i<nbu[stage]; i++)
		{
		ubu[i] = -ubu[i];
		}

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_IDXB(int stage, int *idxb, struct OCP_QP *qp)
	{
	// extract dim
	int *nb = qp->dim->nb;

	int ii;
	for(ii=0; ii<nb[stage]; ii++)
		qp->idxb[stage][ii] = idxb[ii];

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_IDXB(int stage, struct OCP_QP *qp, int *idxb)
	{
	// extract dim
	int *nb = qp->dim->nb;

	int ii;
	for(ii=0; ii<nb[stage]; ii++)
		idxb[ii] = qp->idxb[stage][ii];

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_LB(int stage, REAL *lb, struct OCP_QP *qp)
	{
	// extract dim
	int *nb = qp->dim->nb;

	CVT_VEC2STRVEC(nb[stage], lb, qp->d+stage, 0);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_LB(int stage, struct OCP_QP *qp, REAL *lb)
	{
	// extract dim
	int *nb = qp->dim->nb;

	int i;

	CVT_STRVEC2VEC(nb[stage], qp->d+stage, 0, lb);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_UB(int stage, REAL *ub, struct OCP_QP *qp)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;

	CVT_VEC2STRVEC(nb[stage], ub, qp->d+stage, nb[stage]+ng[stage]);
	VECSC_LIBSTR(nb[stage], -1.0, qp->d+stage, nb[stage]+ng[stage]);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_UB(int stage, struct OCP_QP *qp, REAL *ub)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;

	int i;

	CVT_STRVEC2VEC(nb[stage], qp->d+stage, nb[stage]+ng[stage], ub);
	for(i=0; i<nb[stage]; i++)
		{
		ub[i] = -ub[i];
		}

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_C(int stage, REAL *C, struct OCP_QP *qp)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;
	int *ng = qp->dim->ng;

	CVT_TRAN_MAT2STRMAT(ng[stage], nx[stage], C, ng[stage], qp->DCt+stage, nu[stage], 0);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_C(int stage, struct OCP_QP *qp, REAL *C)
	{
	// extract dim
	int *nx = qp->dim->nx;
	int *nu = qp->dim->nu;
	int *ng = qp->dim->ng;

	CVT_TRAN_STRMAT2MAT(nx[stage], ng[stage], qp->DCt+stage, nu[stage], 0, C, ng[stage]);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_D(int stage, REAL *D, struct OCP_QP *qp)
	{
	// extract dim
	int *nu = qp->dim->nu;
	int *ng = qp->dim->ng;

	CVT_TRAN_MAT2STRMAT(ng[stage], nu[stage], D, ng[stage], qp->DCt+stage, 0, 0);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_D(int stage, struct OCP_QP *qp, REAL *D)
	{
	// extract dim
	int *nu = qp->dim->nu;
	int *ng = qp->dim->ng;

	CVT_TRAN_STRMAT2MAT(nu[stage], ng[stage], qp->DCt+stage, 0, 0, D, ng[stage]);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_LG(int stage, REAL *lg, struct OCP_QP *qp)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;

	CVT_VEC2STRVEC(ng[stage], lg, qp->d+stage, nb[stage]);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_LG(int stage, struct OCP_QP *qp, REAL *lg)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;

	CVT_STRVEC2VEC(ng[stage], qp->d+stage, nb[stage], lg);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_UG(int stage, REAL *ug, struct OCP_QP *qp)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;

	CVT_VEC2STRVEC(ng[stage], ug, qp->d+stage, 2*nb[stage]+ng[stage]);
	VECSC_LIBSTR(ng[stage], -1.0, qp->d+stage, 2*nb[stage]+ng[stage]);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_UG(int stage, struct OCP_QP *qp, REAL *ug)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;

	int i;

	CVT_STRVEC2VEC(ng[stage], qp->d+stage, 2*nb[stage]+ng[stage], ug);
	for(i=0; i<ng[stage]; i++)
		{
		ug[i] = -ug[i];
		}

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_ZL(int stage, REAL *Zl, struct OCP_QP *qp)
	{
	// extract dim
	int *ns = qp->dim->ns;

	CVT_VEC2STRVEC(ns[stage], Zl, qp->Z+stage, 0);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_ZL(int stage, struct OCP_QP *qp, REAL *Zl)
	{
	// extract dim
	int *ns = qp->dim->ns;

	CVT_STRVEC2VEC(ns[stage], qp->Z+stage, 0, Zl);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_ZU(int stage, REAL *Zu, struct OCP_QP *qp)
	{
	// extract dim
	int *ns = qp->dim->ns;

	CVT_VEC2STRVEC(ns[stage], Zu, qp->Z+stage, ns[stage]);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_ZU(int stage, struct OCP_QP *qp, REAL *Zu)
	{
	// extract dim
	int *ns = qp->dim->ns;

	CVT_STRVEC2VEC(ns[stage], qp->Z+stage, ns[stage], Zu);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_ZLVEC(int stage, REAL *zl, struct OCP_QP *qp)
	{
	// extract dim
	int *nu = qp->dim->nu;
	int *nx = qp->dim->nx;
	int *ns = qp->dim->ns;

	CVT_VEC2STRVEC(ns[stage], zl, qp->rqz+stage, nu[stage]+nx[stage]);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_ZLVEC(int stage, struct OCP_QP *qp, REAL *zl)
	{
	// extract dim
	int *nu = qp->dim->nu;
	int *nx = qp->dim->nx;
	int *ns = qp->dim->ns;

	CVT_STRVEC2VEC(ns[stage], qp->rqz+stage, nu[stage]+nx[stage], zl);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_ZUVEC(int stage, REAL *zu, struct OCP_QP *qp)
	{
	// extract dim
	int *nu = qp->dim->nu;
	int *nx = qp->dim->nx;
	int *ns = qp->dim->ns;

	CVT_VEC2STRVEC(ns[stage], zu, qp->rqz+stage, nu[stage]+nx[stage]+ns[stage]);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_ZUVEC(int stage, struct OCP_QP *qp, REAL *zu)
	{
	// extract dim
	int *nu = qp->dim->nu;
	int *nx = qp->dim->nx;
	int *ns = qp->dim->ns;

	CVT_STRVEC2VEC(ns[stage], qp->rqz+stage, nu[stage]+nx[stage]+ns[stage], zu);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_IDXS(int stage, int *idxs, struct OCP_QP *qp)
	{
	// extract dim
	int *ns = qp->dim->ns;

	int ii;
	for(ii=0; ii<ns[stage]; ii++)
		qp->idxs[stage][ii] = idxs[ii];

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_IDXS(int stage, struct OCP_QP *qp, int *idxs)
	{
	// extract dim
	int *ns = qp->dim->ns;

	int ii;
	for(ii=0; ii<ns[stage]; ii++)
		idxs[ii] = qp->idxs[stage][ii];

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_LS(int stage, REAL *ls, struct OCP_QP *qp)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;
	int *ns = qp->dim->ns;

	CVT_VEC2STRVEC(ns[stage], ls, qp->d+stage, 2*nb[stage]+2*ng[stage]);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_LS(int stage, struct OCP_QP *qp, REAL *ls)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;
	int *ns = qp->dim->ns;

	CVT_STRVEC2VEC(ns[stage], qp->d+stage, 2*nb[stage]+2*ng[stage], ls);

	return;
	}



void CVT_COLMAJ_TO_OCP_QP_US(int stage, REAL *us, struct OCP_QP *qp)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;
	int *ns = qp->dim->ns;

	CVT_VEC2STRVEC(ns[stage], us, qp->d+stage, 2*nb[stage]+2*ng[stage]+ns[stage]);

	return;
	}



void CVT_OCP_QP_TO_COLMAJ_US(int stage, struct OCP_QP *qp, REAL *us)
	{
	// extract dim
	int *nb = qp->dim->nb;
	int *ng = qp->dim->ng;
	int *ns = qp->dim->ns;

	int i;

	CVT_STRVEC2VEC(ns[stage], qp->d+stage, 2*nb[stage]+2*ng[stage]+ns[stage], us);

	return;
	}



void CHANGE_BOUNDS_DIMENSIONS_OCP_QP(int *nbu, int *nbx, struct OCP_QP *qp)
	{
		// TODO runtime check that new memsize is smaller or equal than old
		int N = qp->dim->N;
		int *nb = qp->dim->nb;
		int *ng = qp->dim->ng;
		int *ns = qp->dim->ns;

		int ii, jj;

		char *c_ptr;
		c_ptr = (char *) qp->d->pa;

	for(ii=0; ii<=N; ii++)
		{
		qp->dim->nbu[ii] = nbu[ii];
		qp->dim->nbx[ii] = nbx[ii];
		nb[ii] = nbu[ii] + nbx[ii];
		}

	for(ii=0; ii<=N; ii++)
		{
		CREATE_STRVEC(2*nb[ii]+2*ng[ii]+2*ns[ii], qp->d+ii, c_ptr);
		c_ptr += nb[ii]*sizeof(REAL); // lb
		c_ptr += ng[ii]*sizeof(REAL); // lg
		c_ptr += nb[ii]*sizeof(REAL); // ub
		c_ptr += ng[ii]*sizeof(REAL); // ug
		c_ptr += ns[ii]*sizeof(REAL); // ls
		c_ptr += ns[ii]*sizeof(REAL); // us
		}

	return;

	}


