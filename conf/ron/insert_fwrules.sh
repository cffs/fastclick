#!/bin/tcsh

sudo ipfw add 750 divert 3000 udp from any 4000 to me 4000 in recv fxp0
sudo ipfw add 750 divert 3001 tcp from any to me 50000-50999 in recv fxp0
sudo ipfw add 750 divert 4000 tcp from $HOST 49900-49999 to any out xmit fxp0
sudo ipfw add 750 divert 4001 tcp from any to $HOST 49900-49999 in recv fxp0
sudo ipfw add 750 divert 4002 udp from any 4001 to $HOST 4001 in recv fxp0


