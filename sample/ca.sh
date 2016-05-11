#!/bin/sh
export DIR_BASE=/home/jitumoto/work/src/popstest
export EGC_JID=0
export EGC_CONFIG=${DIR_BASE}sample/config
${DIR_BASE}/sample/clientA 1> ${DIR_BASE}/sample/ca.out 2> ${DIR_BASE}/sample/ca.err

