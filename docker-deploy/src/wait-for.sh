#!/bin/sh

set -e

host="$1"
port="$2"
shift
shift
cmd="$@"

until nc -z "$host" "$port"; do
  >&2 echo "Postgres is unavailable - waiting"
  sleep 1
done

>&2 echo "Postgres is up - executing command"
exec $cmd
