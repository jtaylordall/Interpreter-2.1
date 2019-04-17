#include "myInterpreter.h"

using namespace std;

//Constructor
myInterpreter::myInterpreter(vector<Token> &v_in){
  v = v_in; //reads in vector of tokens
  i = 0;
  readin(); //automatically reads tokens into database
}

myInterpreter::~myInterpreter(){
  int rlSize = rule_list.size();
  for(int a = 0; a < rlSize; a++){
    delete node_list[a].second;
  }
}


//Database
void myInterpreter::readin(){//Reads Tokens into the Database
  int vSize = v.size();
  vector<Token> v_schemes;
  while(i < vSize && v.at(i).getTypeint() != _eof){
    readschemes(); //builds database
    readfacts();
    readrules();
    readqueries(); //interprets queries according to contents of database
    break;
  }
  print_query(); //outputs formatted interpreter data
  return;
}

void myInterpreter::readschemes(){//Creates Relations from Schemes, called by readin()
  i++; //Skip Scheme;
  i++; //Skip colon;
  while(v.at(i).getTypeint() != _facts){
    string name = v.at(i).getValue(); //read in name of predicate
    i++; //moves past it
    vector<string> sch;
    if(v.at(i).getTypeint() == _left_paren){
      i++; //Skip left paren
      while(v.at(i).getTypeint() != _right_paren){ //reads in parameters, skiping commas
        if(v.at(i).getTypeint() == _id){
          sch.push_back(v.at(i).getValue());
        }
        i++; //iterates to next token (id, commas, right paren)
      }
    }
    Scheme s(sch);
    Relation r(name, s);
    db.addRelation(r); //adds scheme to database with a unique relation
    i++;
  }
  return;
}

void myInterpreter::readfacts(){//Creates Tuples from Facts, adds Tuples to appropriate Relation, called by readin()
  i++; //Skip Facts;
  i++; //Skip colon;
  string name;
  std::map<string, Relation>::iterator it;
  while(v.at(i).getTypeint() != _rules){
    name = v.at(i).getValue(); //gets predicate name of fact
    it = db.find(name); //finds relation with name matching the fact
    i++;

    vector<string> tup;
    if(v.at(i).getTypeint() == _left_paren){ //reads in tuples and stores them as a tuple
      i++;
      while(v.at(i).getTypeint() != _right_paren){
        if(v.at(i).getTypeint() == _id || v.at(i).getTypeint() == _string){
          tup.push_back(v.at(i).getValue());
        }
        i++;
      }
    }
    Tuple t(tup);
    it->second.addTuple(t); //adds tuple to the relation
    i++; //skip period
    i++; //iterates to next id (predicate name) or to rules
  }
  return;
}


//Rule Interpreter
void myInterpreter::readrules(){//Skips past all rules, called by readin()
  i++; //Skip Rules;
  i++; //Skip colon;

  int counter = 0;
  while(v.at(i).getTypeint() != _queries){
    stringstream r_string;

    //Read head predicate name
    string hp_name = v.at(i).getValue();
    Node* n = new Node(hp_name, counter);
    map<string, Relation>::iterator it;
    it = db.find(hp_name);
    Relation hp = it->second;
    r_string << v.at(i).getValue();
    i++;

    //Read head predicate's parameters
    vector<string> hp_params;
    if(v.at(i).getTypeint() == _left_paren){ //reads in tuples and stores them as a tuple
      r_string << v.at(i).getValue();
      i++;
      while(v.at(i).getTypeint() != _right_paren){
        if(v.at(i).getTypeint() == _id){
          hp_params.push_back(v.at(i).getValue());
        }
        r_string << v.at(i).getValue();
        i++;
      }
      r_string << v.at(i).getValue();
      i++;
    }
    Scheme hp_params_s(hp_params);
    Relation forhpvec(hp_name, hp_params_s, counter);
    //Read _colon_dash
    r_string << " " << v.at(i).getValue() << " ";
    i++;

    //Read through all right-side predicates
    set<Relation> db_rules;
    int counter2 = 0;
    vector<string> deps;
    deps.push_back(hp_name);
    while(v.at(i).getTypeint() != _period){
      string predname = v.at(i).getValue();
      deps.push_back(predname);
      n->addDependency_str(predname);
      it = db.find(predname);
      Relation pred = it->second;
      r_string << predname;
      i++;
      vector<string> pred_params;
      while(v.at(i).getTypeint() != _right_paren){
        if(v.at(i).getTypeint() == _id || v.at(i).getTypeint() == _string){
          pred_params.push_back(v.at(i).getValue());
        }
        r_string << v.at(i).getValue();
        i++;
      }
      r_string << v.at(i).getValue();
      i++;
      Tuple pred_t(pred_params);
      Relation local = assessquery(pred_t, pred);
      local.addOrder(counter2);
      local.changename(predname);
      db_rules.insert(local);
      if(v.at(i).getTypeint() == _comma){
        r_string << v.at(i).getValue();
        i++;
      }
      counter2++;
    }
    mapofrules.insert(pair<Relation,set<Relation>>(forhpvec, db_rules));
    node_list.push_back(pair<vector<string>,Node*>(deps,n));
    r_string << v.at(i).getValue();
    rule_list.push_back(r_string.str());
    r_string.str("");
    i++;
    counter++;
  }
  buildGraph();
  printGraph();
  buildReverseGraph();
  dfsForest_reverse();
  dfsForest();
  fixedpoint_optimized();
  return;
}



void myInterpreter::fixedpoint_optimized(){
  int passes = 0;
  cout << "Rule Evaluation" << endl;
  vector<Relation> output_r;

  set<Node*>::iterator snit;
  map<Relation,set<Relation>>::iterator rit;

  int counter = 0;

  int slSize = scc_list.size();
  for(int a = 0; a < slSize; a++){
    int tempcounter = 0;
    int sccSize = scc_list[a].size();
    vector<int> sccPos;
    stringstream ss;

    for(snit = scc_list[a].begin(); snit != scc_list[a].end(); snit++){
      ss << "R"<< (*snit)->getOrder();
      sccPos.push_back((*snit)->getOrder());
      if(tempcounter < sccSize - 1){
        ss << ",";
      }
      else{
        ss << endl;
      }
      tempcounter++;
    }
    string outstring = ss.str();
    cout << "SCC: " << outstring;
    if(scc_list[a].size() == 1){
      snit = scc_list[a].begin();
      map<int,Node*> dependencies = (*snit)->getDependencies();
      bool selfdependency = false;

      for(map<int,Node*>::iterator sit = dependencies.begin(); sit != dependencies.end(); sit++){
        if(sit->first == (*snit)->getOrder()){
          selfdependency = true;
        }
      }

      if(!selfdependency){
        passes = 0;
        rit = mapofrules.begin();
        advance(rit, (*snit)->getOrder());
        Relation result;
        Relation temp_rule = rit->first;
        set<Relation> temp = rit->second;
        int tempSize = temp.size();
        if (tempSize > 1){
          int b = 0;
          for(set<Relation>::iterator brit = temp.begin(); brit != temp.end(); brit++){ //iterate through relations of the rule
            Relation rhere = *brit;
            mmit = db.find(rhere.getName());
            Tuple sh(rhere.getScheme());
            Scheme temp_s(rhere.getScheme());
            if(b < tempSize - 1 && brit != temp.begin()){
              Relation tojoin = mmit->second;
              tojoin = assessquery(sh, tojoin);
              tojoin = tojoin.rename(temp_s);
              int aaa = temp_rule.getScheme().size() - (result.getScheme().size() + tojoin.getScheme().size());
              result = result.join(tojoin, aaa);
            }
            else if(brit == temp.begin()){
              result = mmit->second;
              result = assessquery(sh, result);
              result = result.rename(temp_s);
            }
            else{
              Relation tojoin = mmit->second;

              int aaa = temp_rule.getScheme().size() - (result.getScheme().size() + tojoin.getScheme().size());

              tojoin = assessquery(sh, tojoin);
              tojoin = tojoin.rename(temp_s);
              result = result.join(tojoin,aaa);
              vector<int> pos = getPositions4project(temp_rule.getScheme(), result.getScheme());
              result = result.project(pos);
              result.changename(temp_rule.getName());
              map<string, Relation>::iterator it2 = db.find(temp_rule.getName());
              Relation u = it2->second.unionize(result);
              u.addOrder(counter);
              output_r.push_back(u);
            }
            b++;
          }
        }
        else{
          Relation rhere = *temp.begin();
          mmit = db.find(rhere.getName());
          result = mmit->second;
          Tuple sh(rhere.getScheme());
          result = assessquery(sh, result);
          Scheme temp_s(rhere.getScheme());
          result = result.rename(temp_s);
          vector<int> pos = getPositions4project(temp_rule.getScheme(), result.getScheme());
          result = result.project(pos);
          result.changename(temp_rule.getName());
          map<string, Relation>::iterator it2 = db.find(temp_rule.getName());
          Relation u = it2->second.unionize(result);
          u.addOrder(counter);
          output_r.push_back(u);
        }
        counter++;
        passes++;
        print_rules2(output_r,sccPos);
        output_r.clear();
        sccPos.clear();
        cout << passes << " passes: " << outstring;
      }


      else{
        //assess in a fixedpoint algorithm
        passes = 0;
        bool doagain2 = true;
        while(doagain2){
          doagain2 = false;
          rit = mapofrules.begin();
          advance(rit, (*snit)->getOrder());
          Relation result;
          Relation temp_rule = rit->first;
          set<Relation> temp = rit->second;
          int tempSize = temp.size();
          if (tempSize > 1){
            int b = 0;
            for(set<Relation>::iterator brit = temp.begin(); brit != temp.end(); brit++){ //iterate through relations of the rule
              Relation rhere = *brit;
              mmit = db.find(rhere.getName());
              Tuple sh(rhere.getScheme());
              Scheme temp_s(rhere.getScheme());
              if(b < tempSize - 1 && brit != temp.begin()){
                Relation tojoin = mmit->second;
                tojoin = assessquery(sh, tojoin);
                tojoin = tojoin.rename(temp_s);
                int aaa = temp_rule.getScheme().size() - (result.getScheme().size() + tojoin.getScheme().size());
                result = result.join(tojoin, aaa);
              }
              else if(brit == temp.begin()){
                result = mmit->second;
                result = assessquery(sh, result);
                result = result.rename(temp_s);
              }
              else{
                Relation tojoin = mmit->second;
                tojoin = assessquery(sh, tojoin);
                tojoin = tojoin.rename(temp_s);
                int aaa = temp_rule.getScheme().size() - (result.getScheme().size() + tojoin.getScheme().size());
                result = result.join(tojoin,aaa);
                vector<int> pos = getPositions4project(temp_rule.getScheme(), result.getScheme());
                result = result.project(pos);
                result.changename(temp_rule.getName());
                map<string, Relation>::iterator it2 = db.find(temp_rule.getName());
                Relation u = it2->second.unionize(result);
                u.addOrder(counter);
                output_r.push_back(u);
                if(u.getSize() > 0){
                  doagain2 = true;
                }
              }
              b++;
            }
          }
          else{
            Relation rhere = *temp.begin();
            mmit = db.find(rhere.getName());
            result = mmit->second;
            Tuple sh(rhere.getScheme());
            result = assessquery(sh, result);
            Scheme temp_s(rhere.getScheme());
            result = result.rename(temp_s);
            vector<int> pos = getPositions4project(temp_rule.getScheme(), result.getScheme());
            result = result.project(pos);
            result.changename(temp_rule.getName());
            map<string, Relation>::iterator it2 = db.find(temp_rule.getName());
            Relation u = it2->second.unionize(result);
            u.addOrder(counter);
            output_r.push_back(u);
            if(u.getSize() > 0){
              doagain2 = true;
            }
          }
          int sccPosSize = sccPos.size();
          if(counter > sccPosSize){
            for(int bb = 0; bb < sccPosSize; bb++){
              sccPos.push_back(sccPos[bb]);
            }
            for(int bb = 0; bb < sccPosSize; bb++){
              sccPos.push_back(sccPos[bb]);
            }
          }
          counter++;
          passes++;
        }
        print_rules2(output_r,sccPos);
        output_r.clear();
        sccPos.clear();
        cout << passes << " passes: " << outstring;
      }
    }


    else{ //fixedpoint for multiple rules
      passes = 0;
      bool doagain2 = true;
      while(doagain2){
        doagain2 = false;
        for(snit = scc_list[a].begin(); snit != scc_list[a].end(); snit++){
          rit = mapofrules.begin();
          advance(rit, (*snit)->getOrder());
          Relation result;
          Relation temp_rule = rit->first;
          set<Relation> temp = rit->second;
          int tempSize = temp.size();
          if (tempSize > 1){
            int b = 0;
            for(set<Relation>::iterator brit = temp.begin(); brit != temp.end(); brit++){ //iterate through relations of the rule
              Relation rhere = *brit;
              mmit = db.find(rhere.getName());
              Tuple sh(rhere.getScheme());
              Scheme temp_s(rhere.getScheme());
              if(b < tempSize - 1 && brit != temp.begin()){
                Relation tojoin = mmit->second;
                tojoin = assessquery(sh, tojoin);
                tojoin = tojoin.rename(temp_s);
                int aaa = temp_rule.getScheme().size() - (result.getScheme().size() + tojoin.getScheme().size());
                result = result.join(tojoin,aaa);
              }
              else if(brit == temp.begin()){
                result = mmit->second;
                result = assessquery(sh, result);
                result = result.rename(temp_s);
              }
              else{
                Relation tojoin = mmit->second;
                tojoin = assessquery(sh, tojoin);
                tojoin = tojoin.rename(temp_s);
                int aaa = temp_rule.getScheme().size() - (result.getScheme().size() + tojoin.getScheme().size());
                result = result.join(tojoin, aaa);
                vector<int> pos = getPositions4project(temp_rule.getScheme(), result.getScheme());
                result = result.project(pos);
                result.changename(temp_rule.getName());
                map<string, Relation>::iterator it2 = db.find(temp_rule.getName());
                Relation u = it2->second.unionize(result);
                u.addOrder(counter);
                output_r.push_back(u);
                if(u.getSize() > 0){
                  doagain2 = true;
                }
              }
              b++;
            }
          }
          else{
            Relation rhere = *temp.begin();
            mmit = db.find(rhere.getName());
            result = mmit->second;
            Tuple sh(rhere.getScheme());
            result = assessquery(sh, result);
            Scheme temp_s(rhere.getScheme());
            result = result.rename(temp_s);
            vector<int> pos = getPositions4project(temp_rule.getScheme(), result.getScheme());
            result = result.project(pos);
            result.changename(temp_rule.getName());
            map<string, Relation>::iterator it2 = db.find(temp_rule.getName());
            Relation u = it2->second.unionize(result);
            u.addOrder(counter);
            output_r.push_back(u);
            if(u.getSize() > 0){
              doagain2 = true;
            }
          }
          int sccPosSize = sccPos.size();
          if(counter > sccPosSize){
            for(int bb = 0; bb < sccPosSize; bb++){
              sccPos.push_back(sccPos[bb]);
            }
          }
          counter++;
        }
        passes++;
      }
      print_rules(output_r,sccPos);
      output_r.clear();
      sccPos.clear();
      cout << passes << " passes: " << outstring;

    }
  }
  return;
}

vector<int> myInterpreter::getPositions4project(vector<string> hp, vector<string> p){
  vector<int> out;
  int hpSize = hp.size();
  int pSize = p.size();
  for(int a = 0; a < hpSize; a++){
    for(int b = 0; b < pSize; b++){
      if(hp.at(a) == p.at(b)){
        out.push_back(b);
      }
    }
  }
  return out;
}

void myInterpreter::print_rules(vector<Relation>& output_r, vector<int>& in){
  int rSize = output_r.size();
  int inSize = in.size();
  for(int z = 0; z < rSize; z++){ //iterates through output vector
    mmit = db.find(output_r[z].getName());
    output_r[z].addScheme2(mmit->second.getScheme());
    cout << rule_list.at(in.at(z)) << endl;
    output_r[z].print_output_r(); //format-prints each relation in output vector
    if(z >= inSize){
      break;
    }
  }
  return;
}
void myInterpreter::print_rules2(vector<Relation> &output_r, vector<int>& in){
  int rSize = output_r.size();
  for(int z = 0; z < rSize; z++){ //iterates through output vector
    mmit = db.find(output_r[z].getName());
    output_r[z].addScheme2(mmit->second.getScheme());
    cout << rule_list.at(in.at(0)) << endl;
    output_r[z].print_output_r(); //format-prints each relation in output vector
  }
  return;
}


//Query Interpreter
void myInterpreter::readqueries(){//Reads in queries, assesses them, and stores product in output_q vector, called by readin()
  i++; //Skip Queries;
  i++; //Skip colon;
  //map<string, Relation>::iterator it;
  while(v.at(i).getTypeint() != _eof){
    if(v.at(i).getTypeint() == _eof){
      return;
    }
    string name;
    name = v.at(i).getValue(); //stores name of query
    mmit = db.find(name); //finds relation with name matching the query
    i++;
    if(v.at(i).getTypeint() == _eof){
      return;
    }
    vector<string> tup;
    bool lpar = v.at(i).getTypeint() == _left_paren;
    if(lpar){ //reads in parameters
      i++;
      bool rpar = v.at(i).getTypeint() == _right_paren;
      while(!rpar){
        if(v.at(i).getTypeint() == _eof){
          return;
        }
        rpar = v.at(i).getTypeint() == _right_paren;
        int tok_type = v.at(i).getTypeint();
        if(tok_type == _id){
          tup.push_back(v.at(i).getValue());
        }
        else if(tok_type == _string){
          tup.push_back(v.at(i).getValue());

        }
        i++;
        rpar = v.at(i).getTypeint() == _right_paren;
      }
    }
    Tuple t(tup); //stores query parameters in a tuple
    Relation assessed = assessquery(t, mmit->second); //assesses the query and stores results in a relation
    output_q.push_back(assessed); //adds assessed relation to output_q vector
    while(v.at(i).getTypeint() != _id || v.at(i).getTypeint() == _period || v.at(i).getTypeint() != _eof){ //iterate i until new id or eof
      if(v.at(i).getTypeint() == _q_mark){
      }
      i++;
      if(v.at(i).getTypeint() == _eof){
        return;
      }
      else if (v.at(i).getTypeint() == _id){
        break;
      }
    }
    if(v.at(i).getTypeint() == _eof){
      return;
    }
  }
  return;
}

Relation myInterpreter::assessquery(Tuple &t, Relation &r){//runs relational operators on a query, assists readqueries()
  vector<string> t_vec = t.getTuple();
  Relation local = r;

  //Step 1: Rename according to query
  Scheme sv(t.getTuple());
  local = local.rename(sv); //rename to be assessed relation according to query parameters

  //Step 2: Find positions of parameters
  map<string, vector<int>> linked = checkSameness(t); //find any duplicates variables or constants in query
  vector<int> varpos = varPositions(t.getTuple()); //finds positions of variables in the query

  //Step 3: Select according to constants
  //Constants
  for(map<string, vector<int>>::iterator it = linked.begin(); it != linked.end(); it++){ //performs selects on constants
    if((int)it->first.at(0) == 39){ //checks to see if string starts with single quote
      int msize = it->second.size();
      for(int a = 0; a < msize; a++) //iterates through constant's position vector
      {
        local = local.select_pv(it->second.at(a), it->first); //performs select based on value and postion
      }
    }
  }

  //Step 4: Select according to variables
  //Variables
  if(varpos.size() > 0){ //only assess if there are variables
    for(map<string, vector<int>>::iterator it = linked.begin(); it != linked.end(); it++){ //performs selects on variables
      if((int)it->first.at(0) != 39) //checks to make sure string doesn't start with single quote
      {
        local = local.select_pp(it->second); //performs select based on position vector
      }
    }

    //Step 4.5: project according to variable positions
    varpos = removeDuplicates_pos(t.getTuple()); //finds positions of variables after removing duplicates
    local = local.project(varpos); //projects columns linked to variables
  }

  //Step 5: Return new assessed relation
  local.addQuery(t); //stores query in relation, for outputing
  return local; //returns query-assessed relation
}

map<string,vector<int>> myInterpreter::checkSameness(Tuple &t){//evaluates common parameters in aquery, assists assessquery()
  map<string, vector<int>> m;
  vector<string> t_vec = t.getTuple();
  int t_vecSize = t_vec.size();
  vector<int> positions;

  //put strings from t into a map
  for(int a = 0; a < t_vecSize; a++){
    m.insert(pair<string, vector<int>>(t_vec.at(a), positions));
  }

  //find positions of t strings and stores in its associated vector
  for(map<string, vector<int>>::iterator it = m.begin(); it != m.end(); it++){
    for(int a = 0; a < t_vecSize; a++){ //iterates through tuple's vector
    if(it->first ==t_vec.at(a)){ //if the variable/constant matches the tuples value then stores position
      it->second.push_back(a);
    }
  }
}
return m; //returns map containing variables with a vector of their respective positions
}

vector<int> myInterpreter::varPositions(vector<string> vsch){//finds and returns position of variables within a query, assists assessquery()
  vector<int> vout;
  int vschSize = vsch.size();
  for(int a = 0; a < vschSize; a++){ //iterates through vector of strings
    if(vsch.at(a).at(0) != 39){ //if element is a variable (doesn't start with single quote), stores position
    vout.push_back(a);
  }
}
return vout;
}

vector<int> myInterpreter::removeDuplicates_pos(vector<string> in){//returns positions of vector without duplicates, assists projection stage in assessquery()
  set<string> out;
  vector<int> out_int;
  int inSize = in.size();
  for(int a = 0; a < inSize; a++){ //iterates through vector in
    if(out.count(in.at(a)) == 0){
      out.insert(in.at(a));
      out_int.push_back(a);
    }
  }
  return out_int; //returns vector of positions excluding duplicates
}

void myInterpreter::print_query(){//iterates through output_q vector and format-prints each Relation, called by readin()
  int outputSize = output_q.size();
  cout << "\nQuery Evaluation" << endl;
  for(int a = 0; a < outputSize; a++){ //iterates through output vector
    output_q.at(a).print_output_q(); //format-prints each relation in output vector
  }
  return;
}

void myInterpreter::printGraph(){
  int nlSize = node_list.size();
  cout << "Dependency Graph" << endl;
  for(int a = 0; a < nlSize; a++){
    cout << node_list[a].second->toString_graph();
  }
  cout << endl;
  return;
}

void myInterpreter::printmap(){
  int nlSize = node_list.size();
  cout << "\n\tGraph" << endl;
  for(int a = 0; a < nlSize; a++){
    cout << node_list[a].second->toString_all() << endl;
  }
  return;
}

void myInterpreter::printmap_reverse(){
  int nlSize = node_list.size();
  cout << "\n\tReverse Graph" << endl;
  for(int a = 0; a < nlSize; a++){
    cout << node_list[a].second->toString_all_reverse() << endl;
  }
  return;
}

void myInterpreter::printmap_postdfs(){
  int nlSize = node_list.size();
  cout << "\n\tPost-DFS" << endl;
  for(int a = 0; a < nlSize; a++){
    cout << node_list[a].second->toString_all_dfs() << endl;
  }
  return;
}

void myInterpreter::buildGraph(){
  int nlSize = node_list.size();
  for(int k = 0; k < nlSize; k++){ //refers to node a
    vector<string> vs = node_list[k].second->getDependencies_str();
    int vsSize = vs.size();
    if(cyclesToSelf(node_list[k].first)){
      node_list[k].second->addDependency(node_list[k].second);
    }
    for(int a = 0; a < vsSize; a++){//refers to dependencies of node a
      string tofind = vs.at(a);
      for(int l = 0; l < nlSize; l++){ //refers to node b
        if(k != l){
          if(node_list[l].second->getName() == tofind){
            node_list[k].second->addDependency(node_list[l].second);
          }
        }
      }
    }
  }
  return;
}

void myInterpreter::buildReverseGraph(){
  int nlSize = node_list.size();
  for(int a = 0; a < nlSize; a++){
    map<int,Node*> deps = node_list[a].second->getDependencies();
    for(map<int,Node*>::iterator it = deps.begin(); it != deps.end(); it++){
      it->second->addReverseDependency(node_list[a].second);
    }
  }
  return;
}


bool myInterpreter::cyclesToSelf(vector<string> in){
  int inSize = in.size();
  if(inSize > 1){
    string comp = in.at(0);
    for(int a = 1; a < inSize; a++){
      if(comp == in.at(a)){
        return true;
      }
    }
  }
  return false;
}

//dfs stuff
void myInterpreter::dfs_reverse(Node *in){
  in->markVisited();
  map<int,Node*> dependencies = in->getReverseDependencies();
  for(map<int,Node*>::iterator sit = dependencies.begin(); sit != dependencies.end(); sit++){
    if(sit->second->isVisited() == false){
      dfs_reverse(sit->second);
    }
  }
  postorder.push(in);
  return;
}

void myInterpreter::dfsForest_reverse(){
  int nlSize = node_list.size();
  for(int a = 0; a < nlSize; a++){
    node_list[a].second->unmarkVisited();
  }
  for(int a = 0; a < nlSize; a++){
    if(node_list[a].second->isVisited() == false){
      dfs_reverse(node_list[a].second);
    }
  }
  return;
}

void myInterpreter::dfsForest(){
  int nlSize = node_list.size();
  for(int a = 0; a < nlSize; a++){
    node_list[a].second->unmarkVisited();
  }
  while(!postorder.empty()){
    if(postorder.top()->isVisited() == false){
      dfs(postorder.top());
      scc_list.push_back(scc);
      scc.clear();
    }
    postorder.pop();
  }
  return;
}

void myInterpreter::dfs(Node *in){
  in->markVisited();
  map<int,Node*> dependencies = in->getDependencies();
  for(map<int,Node*>::iterator sit = dependencies.begin(); sit != dependencies.end(); sit++){
    if(sit->second->isVisited() == false){
      in->addDfsDependency(sit->second);
      dfs(sit->second);
    }
  }
  scc.insert(in);
  return;
}

void myInterpreter::printSCCs(){
  int slSize = scc_list.size();
  for(int a = 0; a < slSize; a++){
    set temp = scc_list[a];
    cout << "SCC: " << a << endl;
    for(set<Node*>::iterator it = temp.begin(); it != temp.end(); it++){
      cout <<(*it)->toString() << endl;
    }
    cout << endl;
  }
  return;
}
