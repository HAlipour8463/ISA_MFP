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

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.4922868.svg)](https://doi.org/10.5281/zenodo.4922868)


 **dimacs_baisc_factors_sorted.c** is a source code in C to extract the basic factors of a given netwrok in DIMACS format. The basic factors are: numNodes, numArcs, NetCap, maxcap, mincap, SrcAAC, SnkAAC,  Src_degree, Snk_degree, AvCap, StDAC, NmBdCap, NmGdCap, PotNetExcess, PotNetDeficit, StDAvNdDg, and many more features that are found in the code. The extracted basic factors then used by **ISAFtrExtractorPool_InitilizeTime_mod_pool.m** to extract the final features used in the instance space analysis for MFP (see https://matilda.unimelb.edu.au/matilda/problems/opt/mfp#mfp).  
**ISA.m** is a main script that control differnt functions such as "ISAFtrExtractorPool_InitilizeTime_mod_pool** to generate the metadata used in the instance space analysis.

**purifyInst.m** is the source for the instance selection algorithm, which takes the metadata generated by ISA and filters instances based on thier similarity and the similarity of the algorithm's perfromances on them. **CVNND.m** calculates number of instances, unifomrity, and ViSA ratio for the critical set of instances returened by purifyInst.m.


csv files are the metadata for MFP; **Metadata_M0.csv** is the initial metadata; **Metadata_M0'.csv** is the augmented metadata obtained by adding new instances to **Metadata_M0.csv**; **Metadata_M2.csv** is resulted after applying the instance selection process on **Metadata_M0'.csv**. For more details on obtaining **Metadata_M0'.csv** and **Metadata_M2.csv**, we refer readers to the following paper.
```
**Enhanced Instance Space Analysis for the Maximum Flow Problem**
by: Hossein Alipour, Mario Andrés Muñoz, Kate Smith-Miles
European Journal of Operational Research
2022
```


