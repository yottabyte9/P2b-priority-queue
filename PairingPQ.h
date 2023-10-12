// Project identifier: 43DE0E0C4C76BFAA6D8C2F5AEAE0518A9C42CF4E

#ifndef PAIRINGPQ_H
#define PAIRINGPQ_H

#include "Eecs281PQ.h"
#include <deque>
#include <utility>

// A specialized version of the '' ADT implemented as a pairing heap.
template<typename TYPE, typename COMP_FUNCTOR = std::less<TYPE>>
class PairingPQ : public Eecs281PQ<TYPE, COMP_FUNCTOR> {
    // This is a way to refer to the base class object.
    using BaseClass = Eecs281PQ<TYPE, COMP_FUNCTOR>;

public:
    // Each node within the pairing heap
    class Node {
    public:
        // TODO: After you add add one extra pointer (see below), be sure to
        // initialize it here.
        explicit Node(const TYPE &val)
            : elt{ val }, child{ nullptr }, sibling{ nullptr }, parent{ nullptr }
        {}

        // Description: Allows access to the element at that Node's
        // position.  There are two versions, getElt() and a dereference
        // operator, use whichever one seems more natural to you.
        // Runtime: O(1) - this has been provided for you.
        const TYPE &getElt() const { return elt; }
        const TYPE &operator*() const { return elt; }

        // The following line allows you to access any private data
        // members of this Node class from within the PairingPQ class.
        // (ie: myNode.elt is a legal statement in PairingPQ's add_node()
        // function).
        friend PairingPQ;

    private:
        TYPE elt;
        Node *child;
        Node *sibling;
        // TODO: Add one extra pointer (parent or previous) as desired.
        Node *parent;
    }; // Node


    // Description: Construct an empty pairing heap with an optional
    //              comparison functor.
    // Runtime: O(1)
    explicit PairingPQ(COMP_FUNCTOR comp = COMP_FUNCTOR()) :
        BaseClass{ comp } {
        root = nullptr;
    } // PairingPQ()


    // Description: Construct a pairing heap out of an iterator range with an
    //              optional comparison functor.
    // Runtime: O(n) where n is number of elements in range.
    template<typename InputIterator>
    PairingPQ(InputIterator start, InputIterator end, COMP_FUNCTOR comp = COMP_FUNCTOR()) :
        BaseClass{ comp } {
        InputIterator it = start;
        while (it != end) {
            push(*it);
            it++;
        }
    } // PairingPQ()


    // Description: Copy constructor.
    // Runtime: O(n)
    PairingPQ(const PairingPQ& other) :
        BaseClass{ other.compare } {
        root = nullptr;
        *this = other;
    } // PairingPQ()


    // Description: Copy assignment operator.
    // Runtime: O(n)
    // TODO: when you implement this function, uncomment the parameter names.
    PairingPQ& operator=(const PairingPQ& rhs) {
        if (this == &rhs) {
            return *this;
        }
        deletepq(root);
        root = copypq(rhs.root);
        return *this;
    } // operator=()


    // Description: Destructor
    // Runtime: O(n)
    ~PairingPQ() {
        deletepq(root);
    } // ~PairingPQ()


    // Description: Assumes that all elements inside the pairing heap are out
    //              of order and 'rebuilds' the pairing heap by fixing the
    //              pairing heap invariant.  You CANNOT delete 'old' nodes
    //              and create new ones!
    // Runtime: O(n)
    virtual void updatePriorities() {
        if(!root){
            return;
        }
        std::deque<Node*> queue;
        queue.push_back(root);
        root = nullptr;

        while(!queue.empty()){
            Node* r = queue.front();
            queue.pop_front();
            if(r->sibling != nullptr){
                queue.push_back(r->sibling);
                r->sibling->parent = nullptr;
                r->sibling = nullptr;
            }
            if(r->child != nullptr){
                queue.push_back(r->child);
                r->child->parent = nullptr;
                r->child = nullptr;
            }
            if(r->parent != nullptr){
                queue.push_back(r->parent);
                r->parent->parent = nullptr;
                r->parent = nullptr;
            }
            root = meld(root, r);
        }
    } // updatePriorities()

    // Description: Add a new element to the pairing heap. This is already
    //              done. You should implement push functionality entirely
    //              in the addNode() function, and this function calls
    //              addNode().
    // Runtime: O(1)
    virtual void push(const TYPE & val) {
        addNode(val);
    } // push()


    // Description: Remove the most extreme (defined by 'compare') element
    //              from the pairing heap.
    // Note: We will not run tests on your code that would require it to pop
    // an element when the pairing heap is empty. Though you are welcome to
    // if you are familiar with them, you do not need to use exceptions in
    // this project.
    // Runtime: Amortized O(log(n))
    virtual void pop() {
        Node *root_holder = root;
        if(!root->child){
            root = nullptr;
            count --;
            delete root_holder;
            return;
        }
        if(!root->child->sibling){
            root = root->child;
            count--;
            delete root_holder;
            return;
        }
        count--;
        
        std::deque<Node*> dq;
        Node *c = root->child;
        while(!c) {
            dq.push_back(c);
            c->parent->sibling = nullptr;
            c = c->sibling;
        }
        int i = 0;
        while (i < int(dq.size()) - 1) {
            meld(dq[i], dq[i + 1]);
            i += 2;
        }
        i -= 2;
        if(i == int(dq.size())-3){
            meld(dq[i], dq[i+2]);
        }
        while(i < 2){
            meld(dq[i], dq[i-2]);
            i -= 2;
        }
        root = dq.front();
        delete root_holder;
    } // pop()


    // Description: Return the most extreme (defined by 'compare') element of
    //              the pairing heap. This should be a reference for speed.
    //              It MUST be const because we cannot allow it to be
    //              modified, as that might make it no longer be the most
    //              extreme element.
    // Runtime: O(1)
    virtual const TYPE &top() const {
        return root->elt;
    } // top()


    // Description: Get the number of elements in the pairing heap.
    // Runtime: O(1)
    virtual std::size_t size() const {
        return count;
    } // size()

    // Description: Return true if the  is empty.
    // Runtime: O(1)
    virtual bool empty() const {
        return !root;
    } // empty()


    // Description: Updates the  of an element already in the pairing
    //              heap by replacing the element refered to by the Node with
    //              new_value.  Must maintain pairing heap invariants.
    //
    // PRECONDITION: The new , given by 'new_value' must be more
    //              extreme (as defined by comp) than the old .
    //
    // Runtime: As discussed in reading material.
    void updateElt(Node* node, const TYPE & new_value) { 
        node->elt = new_value;
        updatePriorities();
    } // updateElt()


    // Description: Add a new element to the pairing heap. Returns a Node*
    //              corresponding to the newly added element.
    // Runtime: O(1)
    // NOTE: Whenever you create a node, and thus return a Node *, you must
    //       be sure to never move or copy/delete that node in the future,
    //       until it is eliminated by the user calling pop(). Remember this
    //       when you implement updateElt() and updatePriorities().
    Node* addNode(const TYPE & val) {
        Node* temp = new Node(val);
        if(root == nullptr){
            root = temp;
        }
        else{
            root = meld(temp, root);
        }
        count++;
        return (temp);
    } // addNode()

    Node* root_node() {
        return root;
    }


private:
    // TODO: Add any additional member variables or member functions you
    // require here.
    // TODO: We recommend creating a 'meld' function (see the Pairing Heap
    // papers).

    // NOTE: For member variables, you are only allowed to add a "root
    //       pointer" and a "count" of the number of nodes. Anything else
    //       (such as a deque) should be declared inside of member functions
    //       as needed.

    Node *root = nullptr;
    size_t count = 0;

    Node *meld(Node *left, Node *right) {
        if(!left) return right;
        if(!right) return left;

        if(!this->compare(left->getElt(), right->getElt())){
            right->sibling = left->child;
            right->parent = left;
            left->child = right;
            return left;
        }
        left->sibling = right->child;
        left->parent = right;
        right->child = left;
        left = right;
        return right;
    }

    
    Node* copypq(Node* node){
        if(!node) return nullptr;
        
        Node *temproot = new Node(node->getElt());
        temproot->sibling = copypq(node->sibling);
        temproot->child = copypq(node->child);
        
        if(temproot->sibling != nullptr){
            temproot->sibling->parent = temproot;
        }
        if(temproot->child != nullptr){
            temproot->child->parent = temproot;
        }
        
        return temproot;
    }

    void deletepq(Node *node){
        if(node != nullptr){
            deletepq(node->sibling);
            deletepq(node->child);
            delete node;
        }
    }
};

#endif // PAIRINGPQ_H
