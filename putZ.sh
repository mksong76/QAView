#!/bin/bash

BATCH=.batch

cat > $BATCH <<EOF
put qaview bin/qaview
put qaview_ko.qm bin/qaview_ko.qm
EOF

sftp -b $BATCH zaurus@zaurus

rm $BATCH
