#pragma once

struct Cell;
struct Cluster;
struct Token;
struct Particle;
struct Entities;

#define FP_PRECISION 0.00001

#define THROW_NOT_IMPLEMENTED() fprintf(stderr,"not implemented"); \
    exit(-1);