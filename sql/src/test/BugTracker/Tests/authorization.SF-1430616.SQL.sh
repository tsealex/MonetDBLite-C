#!/bin/bash

cat << EOF > .monetdb
user=monetdb
password=monetdb
EOF

LANG="en_US.UTF-8"
export LANG

Mlog -x "$MTIMEOUT java nl.cwi.monetdb.client.JdbcClient -e -h $HOST -p $MAPIPORT -d ${TSTDB} -f \"$RELSRCDIR/authorization.SF-1430616-data.sql\""

cat << EOF > .monetdb
user=voc
password=voc
EOF

Mlog -x "$MTIMEOUT java nl.cwi.monetdb.client.JdbcClient -e -h $HOST -p $MAPIPORT -d ${TSTDB} -q"

cat << EOF > .monetdb
user=monetdb
password=monetdb
EOF

Mlog -x "$MTIMEOUT java nl.cwi.monetdb.client.JdbcClient -e -h $HOST -p $MAPIPORT -d ${TSTDB} -f \"$RELSRCDIR/authorization.SF-1430616-drop_user.sql\""

rm -f .monetdb
