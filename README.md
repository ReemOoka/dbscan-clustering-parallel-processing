# ⚙️ Parallel Processing Project: Multithreaded DBSCAN Clustering in C++

This project implements a highly concurrent and memory-efficient version of the **DBSCAN clustering algorithm** in **C++**, designed for parallel execution on large datasets. The goal was to explore parallelism, memory tracking, and synchronization techniques using modern C++ features and concurrency primitives.

Built as part of the **Parallel Computing course at Marist College**, this project showcases efficient use of **threads, mutexes, semaphores, atomics, and memory tracking** in a performance-critical environment.

---

## 📖 Project Summary

The application reads a set of 2D points from input files and performs **DBSCAN clustering** with multithreaded expansion logic. It is optimized to:
- Handle large-scale datasets efficiently
- Limit thread contention using semaphores and atomic variables
- Measure execution time and memory usage at runtime

Output includes clustered points along with detailed performance and memory statistics.

---

## 🚀 Key Features

### 🔄 Parallel Execution
- Each point is processed concurrently using **std::thread**
- Uses a **counting semaphore** to limit the number of active threads (up to 16)
- All cluster expansions occur in parallel, minimizing bottlenecks

### 🧠 Memory Tracking
- Global `new` and `delete` operators are overloaded to track allocated and deallocated memory
- Outputs total memory usage and leaks at the end of execution

### 🔐 Thread Safety
- Uses **`std::atomic`** for shared state (e.g., `clusterId`, `visited` flags)
- Employs **mutex locks** only when necessary to reduce overhead
- Cluster expansion is recursively performed using thread-safe flags

### ⚡ Efficient Clustering Logic
- Uses squared Euclidean distance to avoid `sqrt` calls
- Implements core DBSCAN logic (neighborhood search, minPts, noise detection)
- Skips uninitialized points to reduce memory scanning overhead

---

## 📁 Project Structure

ParallelProcessingProject1/
├── Input files/
│ ├── data_10.txt
│ ├── data_100.txt
│ └── ...
├── project/
│ ├── project.sln
│ ├── project.cpp
│ └── output.txt
├── Instructions on how to compile and run.pdf
├── Parallel Processing Project 1 Report.pdf
└── README.md


---

## 🛠️ Technologies & Concepts Used

| Area                    | Tech/Concept                           | Usage |
|-------------------------|----------------------------------------|-------|
| **Language**            | C++ (Modern, C++17 or later)           | Primary development language |
| **Concurrency**         | `std::thread`, `std::mutex`, `std::atomic`, `std::counting_semaphore` | Thread-safe, lock-minimal parallelism |
| **Memory Tracking**     | `operator new` / `operator delete` overloading | Runtime memory usage reporting |
| **File I/O**            | `std::ifstream`, `std::ofstream`       | Input data parsing and cluster export |
| **Performance Metrics** | `<chrono>`                             | Measures DBSCAN runtime in seconds |
| **Cluster Logic**       | Recursive expansion with neighbor tracking | Implements DBSCAN algorithm in parallel |
| **Optimization**        | Squared distance + heap memory pooling | Reduces unnecessary operations |

---

## 💡 How It Works

1. Load 2D point data from a file (e.g., `data_10000.txt`)
2. Launch a pool of threads to process each point
3. Use DBSCAN to group points into clusters or noise
4. Track time taken and memory used
5. Output results to `output.txt` in the format:
x y cluster_id


---

## 📊 Sample Output

2.3 4.1 1

2.4 4.0 1

10.5 22.0 2

...

Also prints:

DBSCAN runtime: 1.02 seconds

Total points written: 10000

Total allocated memory: 1,234 KB

Total deallocated memory: 1,234 KB

Net memory usage: 0 KB

---

## 🧪 How to Compile & Run

### 🔧 Requirements
- Visual Studio (or any C++17+ compatible compiler)
- Windows OS (for `.sln` file compatibility)

### 🧭 Steps

1. Open `project.sln` in **Visual Studio**
2. Build the solution (`Ctrl+Shift+B`)
3. Run the executable (F5) — or use terminal to run `project.exe`
4. Make sure `data_*.txt` files are in the same directory

Alternatively, compile via command line:
```bash
g++ -std=c++17 -o dbscan project.cpp -pthread
./dbscan
```

---

### 🧠 What You'll Learn
How to parallelize clustering algorithms

How to manage memory in C++ using new, delete, and unique_ptr

Efficient use of atomic operations and thread synchronization

Real-world application of concurrency in data clustering

---

### 📜 License

GNU GENERAL PUBLIC LICENSE

---

### 👩‍💻 Author

Reem Ooka

Full Stack Java Developer|AI/ML Researcher
