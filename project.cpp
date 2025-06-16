#include <iostream>
#include <fstream>
#include <thread>
#include <mutex>
#include <atomic>
#include <semaphore>
#include <memory>  // For dynamic memory allocation using unique_ptr
#include <chrono>  // For measuring time

// Constants for array sizes (can be adjusted)
const int MAX_POINTS = 10000;  // Adjust this to the maximum number of points
const int MAX_NEIGHBORS = 10000;  // Adjust for the maximum neighbors

// Global counters for memory tracking
std::size_t allocatedMemory = 0;
std::size_t deallocatedMemory = 0;

// Struct to represent a 2D point
struct Point {
	double x, y;
	std::atomic<int> clusterId{ 0 };  // Cluster ID, 0 indicates noise
	std::atomic<bool> visited{ false };  // Thread-safe visited flag

	// Default constructor
	Point(double _x = 0, double _y = 0) : x(_x), y(_y) {}
};

// Overload new to track memory allocations
void* operator new(std::size_t size) {
	allocatedMemory += size;
	return malloc(size);
}

// Overload delete to track memory deallocations
void operator delete(void* ptr, std::size_t size) noexcept {
	deallocatedMemory += size;
	free(ptr);
}

// Global array for points and concurrency management
Point points[MAX_POINTS];
std::mutex vector_mutex;  // Mutex to protect shared resources
std::counting_semaphore<16> thread_limiter(16);  // Limit to concurrent threads

// Squared Euclidean distance to avoid using sqrt
double squaredDistance(const Point& a, const Point& b) {
	return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

// Find the neighboring points within epsilon distance
int findNeighbors(const Point& point, double epsilonSquared, std::unique_ptr<int[]>& neighbors, int maxNeighbors) {
	int count = 0;
	for (int i = 0; i < MAX_POINTS && count < maxNeighbors; ++i) {
		if (points[i].x == 0 && points[i].y == 0) continue;  // Skip uninitialized points
		if (squaredDistance(point, points[i]) <= epsilonSquared) {
			neighbors[count++] = i;
		}
	}
	return count;
}

// Recursive function to expand a cluster
void expandCluster(int pointIndex, std::unique_ptr<int[]>& neighbors, int neighborCount, int clusterId, double epsilonSquared, int minPts) {
	points[pointIndex].clusterId = clusterId;  // Assign cluster ID to the point

	// Allocate newNeighbors array on the heap
	std::unique_ptr<int[]> newNeighbors = std::make_unique<int[]>(MAX_NEIGHBORS);

	for (int i = 0; i < neighborCount; ++i) {
		Point& p = points[neighbors[i]];

		// Only expand on points that haven't been visited
		if (!p.visited.exchange(true)) {  // Mark as visited in a thread-safe way
			int newNeighborCount = findNeighbors(p, epsilonSquared, newNeighbors, MAX_NEIGHBORS);
			if (newNeighborCount >= minPts) {
				expandCluster(neighbors[i], newNeighbors, newNeighborCount, clusterId, epsilonSquared, minPts);
			}
		}

		// Assign to the current cluster if it was marked as noise (clusterId == 0)
		if (p.clusterId == 0) {
			p.clusterId = clusterId;
		}
	}
}

// Function to process a point and attempt to expand a cluster
void processPoint(int index, double epsilonSquared, int minPts, std::atomic<int>& nextClusterId) {
	thread_limiter.acquire();  // Limit the number of concurrent threads

	Point& point = points[index];
	if (!point.visited.exchange(true)) {  // Mark the point as visited

		// Allocate neighbors array on the heap
		std::unique_ptr<int[]> neighbors = std::make_unique<int[]>(MAX_NEIGHBORS);
		int neighborCount = findNeighbors(point, epsilonSquared, neighbors, MAX_NEIGHBORS);

		// If the point is a core point (has enough neighbors), form a new cluster
		if (neighborCount >= minPts) {
			int clusterId = ++nextClusterId;  // Safely increment cluster ID
			expandCluster(index, neighbors, neighborCount, clusterId, epsilonSquared, minPts);
		}
	}

	thread_limiter.release();  // Release the thread
}

// Main DBSCAN function
void dbscan(double epsilon, int minPts) {
	std::atomic<int> nextClusterId = 0;  // Cluster IDs start from 1
	std::unique_ptr<std::thread[]> workers = std::make_unique<std::thread[]>(MAX_POINTS);  // Allocate threads dynamically
	double epsilonSquared = epsilon * epsilon;  // Avoid sqrt by working with squared distances

	for (int i = 0; i < MAX_POINTS; ++i) {
		if (points[i].x != 0 || points[i].y != 0) {  // Only process initialized points
			workers[i] = std::thread(processPoint, i, epsilonSquared, minPts, std::ref(nextClusterId));
		}
	}

	// Wait for all threads to finish
	for (int i = 0; i < MAX_POINTS; ++i) {
		if (workers[i].joinable()) {
			workers[i].join();
		}
	}
}
int main() {
	// Load points from file
	std::ifstream file("data_10000.txt");
	if (!file.is_open()) {
		std::cerr << "Error: Could not open input file!" << std::endl;
		return 1;
	}

	double x, y;
	int pointCount = 0;
	while (file >> x >> y && pointCount < MAX_POINTS) {
		// Use placement new to construct Point at the location directly
		new (&points[pointCount]) Point(x, y);  // Construct Point in-place using placement new
		pointCount++;
	}
	file.close();

	if (pointCount == 0) {
		std::cerr << "Error: No points loaded from the file." << std::endl;
		return 1;
	}

	// Start timing
	auto start = std::chrono::high_resolution_clock::now();

	// Run DBSCAN with specified epsilon and MinPts
	dbscan(2.5, 2);  // Adjust epsilon (radius) and MinPts (minimum points) for DBSCAN

	// Stop timing
	auto end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> duration = end - start;
	std::cout << "DBSCAN runtime: " << duration.count() << " seconds" << std::endl;

	// Write the output to the file
	std::ofstream outFile("output.txt");
	if (!outFile.is_open()) {
		std::cerr << "Error: Could not open output file!" << std::endl;
		return 1;
	}

	int writtenPoints = 0;  // Count how many points are written
	// Write points with their cluster IDs to the file
	for (int i = 0; i < MAX_POINTS; ++i) {
		if (points[i].x != 0 || points[i].y != 0) {  // Only output initialized points
			outFile << points[i].x << " " << points[i].y << " " << points[i].clusterId << std::endl;
			writtenPoints++;
		}
	}
	outFile.close();

	std::cout << "Total points written to output file: " << writtenPoints << std::endl;

	// Print memory consumption details in KB
	std::cout << "Total allocated memory: " << allocatedMemory / 1024 << " KB" << std::endl;
	std::cout << "Total deallocated memory: " << deallocatedMemory / 1024 << " KB" << std::endl;
	std::cout << "Net memory usage: " << (allocatedMemory - deallocatedMemory) / 1024 << " KB" << std::endl;

	// Wait for user input to keep the console open
	std::cin.get();  // Keeps console open until you press Enter
	return 0;
}
