// SPDX-License-Identifier: BSD-3-Clause

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "log/log.h"
#include "utils.h"

#define NUM_THREADS		4

static int sum;
static os_graph_t *graph;
static os_threadpool_t *tp;
/* TODO: Define graph synchronization mechanisms. */
pthread_mutex_t lock;

/* TODO: Define graph task argument. */

static void process_node(unsigned int idx)
{
	/* TODO: Implement thread-pool based processing of graph. */
	os_node_t *node = graph->nodes[idx];

	for (unsigned int i = 0; i < node->num_neighbours; i++) {
		//if neighbour is not visited, create task for it and add to threadpool
		if (graph->visited[node->neighbours[i]] == 0) {
			os_task_t *t = create_task((void *)process_node, (void *)graph->nodes[idx]->neighbours[i], NULL);

			enqueue_task(tp, t);
		}
	}

	//if node is not visited, mark it as visited and add its value to sum
	pthread_mutex_lock(&lock);

	if (graph->visited[idx] == 0) {
		graph->visited[idx] = 1;
		sum += node->info;
	}
	pthread_mutex_unlock(&lock);
}

int main(int argc, char *argv[])
{
	FILE *input_file;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s input_file\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	input_file = fopen(argv[1], "r");
	DIE(input_file == NULL, "fopen");

	graph = create_graph_from_file(input_file);

	/* TODO: Initialize graph synchronization mechanisms. */
	tp = create_threadpool(NUM_THREADS);

	pthread_mutex_init(&lock, NULL);

	os_task_t *t = create_task((void *)process_node, (void *)0, NULL);

	enqueue_task(tp, t);

	for (unsigned int i = 1; i < graph->num_nodes; i++) {
		if (graph->visited[i] == 0 && graph->nodes[i]->num_neighbours != 0) {
			os_task_t *t = create_task((void *)process_node, (void *)i, NULL);

			enqueue_task(tp, t);
		}
	}
	wait_for_completion(tp);
	destroy_threadpool(tp);
	pthread_mutex_destroy(&lock);
	printf("%d", sum);
	return 0;
}
