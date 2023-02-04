##### About

Application to manage urban transport system database and answer related queries;

##### Database

Class `TransportGuide` performs managing inside database, constructed with input-json query at `
"base_requests"` key, which includes two types of objects:
* `"type": "Stop"` defines a bus stop - name, geographical coordinates (longitude, latitude) and 
relative road distances to nearby stops;
* `"type": "Bus` defines a bus route - type and a sequence of stops;

##### Queries

Queries to the database are given at `"stat_requests"` key. There are several types:
* `"type": "Stop"` gives information about the stop with a given name and lists passing buses;
* `"type": "Bus"` computes bus route`s length, curvature and more;
* `"type": "Map"` renders an **optimized** .svg map of the given database (see examples);
* `"type": "Route"` computes **fastest** route between stops `from` and `to` with use of Bellman–Ford algorithm,
.svg object displaying resulting route is also rendered (see examples);

##### Examples
