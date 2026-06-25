#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "linked_list.h"

 
Node * add_newNode(Node* head, pid_t new_pid, char *new_path){
	// Your code here

	// Creating the new node we aim to add at the end of the linked list

			Node *new_node=(Node *)malloc(sizeof(Node)); // Dynamically allocating memory for new Node struct in the heap
			// We dynamically allocate memory as if we passed a struct directly to this function, changes made to the struct in the function won't be reflected outside
			// If we simply created a pointer inside function, it is created in local memory and may be used by some other value when this function is done running
			// Above is so that we dont pass a pointer to an address of local varaible in stack, whose value may be changed once this function is finished running
			if (new_node==NULL){
				return head; // Given the case that malloc fails, return the original head in order to preserve the linkedlist
			}

			(*new_node).pid=new_pid;
			(*new_node).path=new_path;
			(*new_node).next=NULL;

			if (head!=NULL){
					Node *cur= head; // Setting current node to be head

					// Looping through the linked list till we find the last node
					while ((*cur).next!=NULL){
						cur= (*cur).next; // Changing current node to next node
					}
					(*cur).next= new_node; //Changing the next value of last node from NULL to a pointer to our new node
					
			}
			else {
					head= new_node; // Setting pointer to head to be a pointer to the new node	
			}
		
			return head; // Linked List ADT always returns a reference to head after adding a new node in this function

}
	


Node * deleteNode(Node* head, pid_t pid){
	// your code here
	if (head==NULL){
		return NULL; // In the event that we want to delete from an empty list
	}
	else{
		// If head is the process we wish to delete
		if((*head).pid==pid){
			Node* head_backup= head; // Storing a backup of the pointer to head so that we can free it
			head= (*head).next; // Setting head to be the next node of current head

			// Freeing head node in order to avoid memory leaks
			free((*head_backup).path); // We need to free the string in head before we free head itself
			free(head_backup); 

			return head; // returning the value of the new head
		}

		// IF the process we want to delete's details are stored in some interior node
		Node *cur= head; // Setting current node to be head
		while(cur!=NULL){
			Node *next_node= (*cur).next; // creating a backup of the next_node
			if(next_node!=NULL && (*next_node).pid==pid){
				// On the event that the next node of current has the same PID as the node we wish to delete
				
				(*cur).next= (*next_node).next; // We set value of the next node of cur to be the next node of its current next node which will be now removed
				// This node will now be removed as there is no node which points to it as its next
				free((*next_node).path); // We need to free the string in head before we free head itself
				free(next_node); // free the memory to avoid a memory leak
				return head; // Exiting once we have deleted required node
			}
			cur= (*cur).next; // Updating cur
		}
	}
	// The case that we do not find a node with given PID
	return head;
}

void printList(Node *node){
	// Here, given the pointer to the head node of the linked list, we have to print out every other node
	while(node!=NULL){
		printf("Node: %d\n", (*node).pid);
		node=(*node).next;
	}
}

int PifExist(Node *node, pid_t pid){
	// Here, given the head node of a list and a PID, we check if a node with a given PID exists in it

		while(node!=NULL){
			if((*node).pid==pid){
				return 1;
			}
			node=(*node).next;
		}
  	return 0;
}

