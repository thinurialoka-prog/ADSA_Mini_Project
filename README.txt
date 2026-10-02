========================================================================
IS2202 Graph Theory Assignment - Problem C: Smart City Transport
========================================================================

PROJECT OVERVIEW:
This project models a future smart city transportation system restricted
exclusively to public transport (bus networks and train networks).
Using graph theory models and algorithms (such as Dijkstra's Shortest Path),
the simulation evaluates network efficiency under varying passenger demands
throughout different times of the day.

PROJECT STRUCTURE:
SmartCityTransport/
│
├── src/
│   ├── main.c        - Application entry point and workflow orchestration
│   ├── graph.c       - Graph data structures (Vertices, Edges, Adjacency Lists)
│   ├── transport.c   - Bus and train route simulation logic
│   ├── passenger.c   - Variable passenger demand model across time periods
│   ├── dijkstra.c    - Shortest path routing algorithm implementation
│   ├── simulation.c  - City transit simulation execution engine
│   └── profiling.c   - Performance timing & system efficiency metrics
│
├── include/
│   ├── graph.h       - Declarations for graph nodes, edges, and graph APIs
│   ├── transport.h   - Declarations for bus/train network structures
│   ├── passenger.h   - Declarations for time-of-day demand distributions
│   ├── dijkstra.h    - Declarations for Dijkstra shortest path search
│   ├── simulation.h  - Declarations for transit simulation engine
│   └── profiling.h   - Declarations for benchmark and efficiency profiling
│
├── README.txt        - Documentation and module breakdown
└── Makefile          - Build configuration script

BUILD & RUN INSTRUCTIONS:
1. Compile the project:
   make

2. Execute the simulator:
   make run
   or
   ./smart_city_sim (or smart_city_sim.exe)

3. Clean build artifacts:
   make clean

MODULE RESPONSIBILITIES:
- graph.h / graph.c:
  Represents city hubs/stations as vertices and transit connections as weighted,
  directed/undirected edges (distance, speed limit, transport type).

- transport.h / transport.c:
  Manages bus lines and train lines, their capacities, dispatch frequencies,
  and integration into the core city graph.

- passenger.h / passenger.c:
  Generates dynamic passenger trip origin-destination pairs and demand profiles
  tailored for Morning Peak, Afternoon, Evening Peak, and Night schedules.

- dijkstra.h / dijkstra.c:
  Computes optimal public transport routes between stations using weighted 
  graph shortest path algorithms.

- simulation.h / simulation.c:
  Runs the end-to-end trip simulation, tracking passenger transit times and
  network congestion.

- profiling.h / profiling.c:
  Measures algorithm execution duration and calculates efficiency metrics for
  evaluating the proposed transport network design.
========================================================================
