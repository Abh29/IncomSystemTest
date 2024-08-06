#! /bin/bash

# Variables
PG_HOST=        # postgres host address
PG_PORT=        # postgres port
PG_USER=        # postgres username
PG_PASSWORD=    # postgres password
PG_DB=          # postgres database name
TABLES=10
TABLE_SIZE=100000
THREADS=150
TIME=300

usage() {
    echo "Usage: $0 {prepare|run-single|run-multi|cleanup}"
    exit 1
}

if [ $# -eq 0 ]; then
    usage
fi

prepare() {
    echo "Preparing the database..."
    sysbench \
      --db-driver=pgsql \
      --pgsql-host="$PG_HOST" \
      --pgsql-port="$PG_PORT" \
      --pgsql-user="$PG_USER" \
      --pgsql-password="$PG_PASSWORD" \
      --pgsql-db="$PG_DB" \
      --tables="$TABLES" \
      --table-size="$TABLE_SIZE" \
      /usr/share/sysbench/oltp_read_write.lua prepare
}

run_single() {
    echo "Running sysbench with a single thread..."
    sysbench \
      --db-driver=pgsql \
      --pgsql-host="$PG_HOST" \
      --pgsql-port="$PG_PORT" \
      --pgsql-user="$PG_USER" \
      --pgsql-password="$PG_PASSWORD" \
      --pgsql-db="$PG_DB" \
      --threads=1 \
      --tables="$TABLES" \
      --table-size="$TABLE_SIZE" \
      --time="$TIME" \
      /usr/share/sysbench/oltp_read_write.lua run
}

run_multi() {
    echo "Running sysbench with multiple threads..."
    sysbench \
      --db-driver=pgsql \
      --pgsql-host="$PG_HOST" \
      --pgsql-port="$PG_PORT" \
      --pgsql-user="$PG_USER" \
      --pgsql-password="$PG_PASSWORD" \
      --pgsql-db="$PG_DB" \
      --threads="$THREADS" \
      --tables="$TABLES" \
      --table-size="$TABLE_SIZE" \
      --time="$TIME" \
      /usr/share/sysbench/oltp_read_write.lua run
}

cleanup() {
    echo "Cleaning up the database..."
    sysbench \
      --db-driver=pgsql \
      --pgsql-host="$PG_HOST" \
      --pgsql-port="$PG_PORT" \
      --pgsql-user="$PG_USER" \
      --pgsql-password="$PG_PASSWORD" \
      --pgsql-db="$PG_DB" \
      --tables="$TABLES" \
      /usr/share/sysbench/oltp_read_write.lua cleanup
}

case "$1" in
    prepare)
        prepare
        ;;
    run-single)
        run_single
        ;;
    run-multi)
        run_multi
        ;;
    cleanup)
        cleanup
        ;;
    *)
        usage
        ;;
esac
