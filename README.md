# ISA_MFP
This repo contains source codes written in C and MATLAB to extract features of a given set of instances for the maximum flow problem (MFP) and select unbiased instances based on their feature values. The selected instances then used by the instance space analysis (ISA) to detect the impact of features on MFP algorithms' performance. 
To apply ISA, we use the toolkit provided by MATILDA (https://matilda.unimelb.edu.au/matilda/) available at: https://github.com/andremun/InstanceSpace. The resulting instance space can be explored at https://matilda.unimelb.edu.au/matilda/problems/opt/mfp#mfp.

* Benchmarks used here are in DIMACS format (see http://archive.dimacs.rutgers.edu/pub/netflow/generators/network/).

```
Code by: Hossein Alipour
         School of Mathematics and Statistics
         The University of Melbourne
         Australia
         2021
         Email: h.alipour@unimelb.edu.au
 
 Copyright: Hossein Alipour
 ```

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.4922854.svg)](https://doi.org/10.5281/zenodo.4922854)


 **dimacs_baisc_factors.c** is a source code in C to extract the basic factors of a given netwrok in DIMACS format. The basic factors are: numNodes, numArcs, NetCap, maxcap, mincap, SrcAAC, SnkAAC,  Src_degree, Snk_degree, and StDAC. The extracted basic factors then used by **ISAFtrExtractorPool_InitilizeTime_mod_pool.m** to extract the final features used in the instance space analysis for MFP (see https://matilda.unimelb.edu.au/matilda/problems/opt/mfp#mfp).  
**ISA.m** is a main script that control differnt functions such as "ISAFtrExtractorPool_InitilizeTime_mod_pool** to generate the metadata used in the instance space analysis.

**purifyInst.m** is the source for the instance selection algorithm, which takes the metadata generated by ISA and filters instances based on thier similarity and the similarity of the algorithm's perfromances on them. **CVNND.m** calculates number of instances, unifomrity, and ViSA ratio for the critical set of instances returened by purifyInst.m.

Excel files are the metadata for MFP.


