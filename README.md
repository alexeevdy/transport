## Transport Guide

##### About

Application to manage urban transport system database and answer related queries;

##### Database

Class `TransportGuide` manages the database, constructed with an input-json query at `
"base_requests"` key, which includes two types of objects:
* `"type": "Stop"` defines a bus stop - name, geographical coordinates (longitude, latitude) and 
relative road distances to nearby stops;
* `"type": "Bus` defines a bus route - its type and a sequence of stops;

##### Queries

Queries to the database are given at `"stat_requests"` key. There are several types:
* `"type": "Stop"` provides information about the stop with a given name and lists passing buses;
* `"type": "Bus"` computes the bus route`s length, curvature and more;
* `"type": "Map"` renders an **optimized** .svg map of the given database (see examples);
* `"type": "Route"` computes the **fastest** route between stops `from` and `to` with the use of Bellman–Ford algorithm,
.svg object displaying the resulting route is also rendered (see examples);

##### Examples
See `./test/svg` directory for .svg rendered files (_view raw_ for the full image); otherwise, look into `./test/png` directory, containing converted _.png_ images. _raw_ - stops are mapped onto the plane acсording to their geographical coordinates. _optimized_ - we give up geographical accuracy to achieve a better-looking image; stops are uniformly distributed across the plane, and some coordinates are compressed into one.
