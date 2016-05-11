#!/bin/sh
DIR_BASE=/home/jitumoto/work/src/popstest
export EGC_JID=1
export EGC_CONFIG=${DIR_BASE}sample/config
${DIR_BASE}/sample/clientB 1> ${DIR_BASE}/sample/cb.out 2> ${DIR_BASE}/sample/cb.err
