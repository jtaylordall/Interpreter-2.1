#include "Node.h"

Node::Node(){
  visited = false;
}

void Node::addReverseDependency(Node* in){
  reverse.insert(pair<int,Node*>(in->getOrder(),in));
  return;
}

void Node::addDependency(Node* in){
  dependencies.insert(pair<int,Node*>(in->getOrder(),in));
  return;
}

void Node::addDfsDependency(Node* in){
  dfs_dependencies.insert(pair<int,Node*>(in->getOrder(),in));
  return;
}

void Node::addDependency_str(string in){
  dep_str.push_back(in);
  return;
}

Node::Node(string name_in){
  name = name_in;
  visited = false;
}

Node::Node(string name_in, int order_in){
  order = order_in;
  name = name_in;
  visited = false;
}

string Node::toString_graph(){
  stringstream s;
  s << "R" << order << ":";
  int counter = 0;
  int depSize = dependencies.size();
  for(it = dependencies.begin(); it != dependencies.end(); it++){
    s << "R" << it->second->getOrder();
    if(counter < depSize - 1){
      s << ",";
    }
    counter++;
  }
  s << endl;
  return s.str();
}

string Node::toString_children(){
  stringstream s;
  s << "R" << order << " = " << name << endl;
  for(it = dependencies.begin(); it != dependencies.end(); it++){
    s << "  " << it->second->toString() << endl;
  }
  s << endl;
  return s.str();
}

string Node::toString_all(){
  stringstream s;
  s << "R" << order << " = " << name << endl;
  s << "Order: " << order << endl;
  s << "Visited: " << visited << endl;
  for(it = dependencies.begin(); it != dependencies.end(); it++){
    s << "  " << it->second->toString() << endl;
  }
  s << endl;
  return s.str();
}

string Node::toString_all_reverse(){
  stringstream s;
  s << "R" << order << " = " << name << endl;
  s << "Order: " << order << endl;
  s << "Visited: " << visited << endl;
  for(it = reverse.begin(); it != reverse.end(); it++){
    s << "  " << it->second->toString() << endl;
  }
  s << endl;
  return s.str();
}

string Node::toString_all_dfs(){
  stringstream s;
  s << "R" << order << " = " << name << endl;
  s << "Order: " << order << endl;
  s << "Visited: " << visited << endl;
  for(it = dfs_dependencies.begin(); it != dfs_dependencies.end(); it++){
    s << "  " << it->second->toString() << endl;
  }
  s << endl;
  return s.str();
}

string Node::toString(){
  stringstream s;
  s << "R" << order << " = " << name;
  return s.str();
}


bool Node::isVisited(){
  return visited;
}

map<int,Node*> Node::getDependencies(){
  return dependencies;
}

map<int,Node*> Node::getReverseDependencies(){
  return reverse;
}

vector<string> Node::getDependencies_str() const{
  return dep_str;
}

bool Node::depExists(string in){
  int depstrSize = dep_str.size();
  for(int a = 0; a < depstrSize; a++){
    if(dep_str.at(a) == in){
      return true;
    }
  }
  return false;
}

void Node::markVisited(){
  visited = true;
  return;
}

void Node::unmarkVisited(){
  visited = false;
  return;
}

int Node::getOrder(){
  return order;
}

string Node::getName(){
  return name;
}
