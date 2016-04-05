/*
 * egclib.h
 *
 *  Created on: 2016/01/16
 *      Author: RyzeVia
 */

#ifndef EGCLIB_H_
#define EGCLIB_H_

int EGC_init(int uid);
/* uid: このジョブにおける、本プロセスのID, ジョブないでの重複不可
 */

int EGC_send(int dest, char* buf, int bufsize);
/* dest: 送信先ジョブでの相手UID
 * buf: 送信バッファ
 * bufsize: バッファサイズ
 */

int EGC_recv(int src, char* buf, int bufsize);
/* src: 受信元ジョブでの相手UID
 * buf: 受信バッファ
 * bufsize: バッファサイズ
 */


#endif /* EGCLIB_H_ */
