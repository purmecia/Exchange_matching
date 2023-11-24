FROM ubuntu:latest

ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update && \
    apt-get install -y build-essential && \
    apt-get install -y netcat postgresql postgresql-contrib libpqxx-dev

WORKDIR /app
COPY . .

EXPOSE 12345

RUN chmod u+r+x run.sh