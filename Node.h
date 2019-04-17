#pragma once

#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <set>
#include <map>
#include <vector>
#include "Relation.h"

using namespace std;

class Node{

private:
  int order;
  string name;

  vector<string> dep_str;
  map<int,Node*> dependencies;
  map<int,Node*> dfs_dependencies;
  map<int,Node*> reverse;
  map<int,Node*>::iterator it;
  int postorder;

  bool visited;

public:
  Node();
  Node(string name_in);
  Node(string name_in, int order_in);
  //~Node();

  void addDependency(Node* in);
  void addDependency_str(string in);
  void addReverseDependency(Node* in);
  void addDfsDependency(Node* in);

  string getName();
  int getOrder();

  Node* getDependency(string name_in);
  Node* getDependency(int order_in);

  map<int,Node*> getDependencies();
  map<int,Node*> getReverseDependencies();
  vector<string> getDependencies_str() const;

  string toString_graph();
  string toString();
  string toString_children();
  string toString_all();
  string toString_all_reverse();
  string toString_all_dfs();

  bool isVisited();
  void markVisited();
  void unmarkVisited();

  bool depExists(string in);

  bool operator <(const Node &right) const{//Compares Tuples by vectors, used for ordering Tuples in a set
    return order < right.order;
  }
};
