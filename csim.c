#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <strings.h>
#include <limits.h>
#include <assert.h>

typedef unsigned long long int memTagAddr;

 typedef struct
  {
    int valid;
    memTagAddr tag;
    int timestamp;
  }  wLine;

  typedef struct
  {
    wLine *lines;
  } Ccache;

  typedef struct
  {
    Ccache *sets;
  } tCache;
  tCache cache;

typedef struct
{
  int s;
  int b;
  int E;
  int S;
  int B;
} cacheParameters;

void printSummary(int hits, int misses, int evictions)
{
  printf("hits:%d misses:%d evictions:%d\n", hits, misses, evictions);
  FILE* output_fp = fopen(".csim_results", "w");
  assert(output_fp);
  fprintf(output_fp, "%d %d %d\n", hits, misses, evictions);
  fclose(output_fp);
}

unsigned long long calcSetiD(cacheParameters cParameters, memTagAddr addr)
{

  unsigned long long temp, setid;
  int tagSize = (64 - (cParameters.s + cParameters.b));
  temp = addr << (tagSize);
  setid = temp >> (tagSize + cParameters.b);
  return setid;
}
int processFile(char *fileName, cacheParameters cParameters)
{

  int i;
  int hitCount = 0;
  int missCount = 0;
  int evictionCount = 0;

  char instruction;
  int size;
  int tStamp = 0;
  int empty = -1;
  int H = 0;

  memTagAddr addr;

  //printf("%d\n",cParameters.S);
 

  cache.sets = malloc(cParameters.S * sizeof(Ccache));

  for (i = 0; i < cParameters.S; i++)
  {
    //printf("test\n")
    cache.sets[i].lines = malloc(sizeof(wLine) * cParameters.E);
  }



  FILE *fp = fopen(fileName, "r");
  if (fp == NULL)
  {
    printf("fail \n");
    exit(EXIT_FAILURE);
  }

  while (fscanf(fp, " %c %llx,%d", &instruction, &addr, &size) == 3)
  {
    int toEvict = 0;
    if (instruction != 'I')
    {
      memTagAddr tagAddr = addr >> (cParameters.s + cParameters.b);
      Ccache set = cache.sets[calcSetiD(cParameters, addr)];
      int low = INT_MAX;
      int e;
      for (e = 0; e < cParameters.E; e++)
      {
        if (set.lines[e].valid == 1)
        {

          if (set.lines[e].tag == tagAddr)
          {
            hitCount++;
            H = 1;
            set.lines[e].timestamp = tStamp;
            tStamp++;
            //printf("test\n")
          }

          else if (set.lines[e].timestamp < low)
          {
            low = set.lines[e].timestamp;
            toEvict = e;
          }
        }

        else if (empty == -1)
        {
          empty = e;
        }
      }
      if (H != 1)
      {
        missCount++;

        if (empty > -1)
        {
          set.lines[empty].valid = 1;
          set.lines[empty].tag = tagAddr;
          set.lines[empty].timestamp = tStamp;
          tStamp++;
        }

        else if (empty < 0)
        {

          set.lines[toEvict].tag = tagAddr;
          set.lines[toEvict].timestamp = tStamp;
          tStamp++;
          evictionCount++;
        }
      }

      if (instruction == 'M')
      {
        hitCount++;
      }
      empty = -1;
      H = 0;

    }
  }


  printSummary(hitCount, missCount, evictionCount);
  return 0;
}

int main(int argc, char **argv)
{
  cacheParameters cParameters;

  bzero(&cParameters, sizeof(cParameters));

  char *fileName;
  cParameters.s = atoi(argv[2]);
  cParameters.E = atoi(argv[4]);
  cParameters.b = atoi(argv[6]);
  fileName = argv[8];

  cParameters.S = (1 << cParameters.s);
  cParameters.B = (1 << cParameters.b);

  processFile(fileName, cParameters);
  return 0;
}