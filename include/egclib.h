/*
 * egclib.h
 *
 *  Created on: 2016/01/16
 *      Author: RyzeVia
 */

#ifndef EGCLIB_H_
#define EGCLIB_H_

int EGC_init(int uid);
/* uid: ���̃W���u�ɂ�����A�{�v���Z�X��ID, �W���u�Ȃ��ł̏d���s��
 */

int EGC_send(int dest, char* buf, int bufsize);
/* dest: ���M��W���u�ł̑���UID
 * buf: ���M�o�b�t�@
 * bufsize: �o�b�t�@�T�C�Y
 */

int EGC_recv(int src, char* buf, int bufsize);
/* src: ��M���W���u�ł̑���UID
 * buf: ��M�o�b�t�@
 * bufsize: �o�b�t�@�T�C�Y
 */


#endif /* EGCLIB_H_ */
