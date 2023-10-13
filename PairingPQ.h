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
        copypq(other);
    } // PairingPQ()


    // Description: Copy assignment operator.
    // Runtime: O(n)
    // TODO: when you implement this function, uncomment the parameter names.
    PairingPQ& operator=(const PairingPQ& rhs) {
        PairingPQ other(rhs);
        std::swap(this->count, other.count);
        std::swap(this->root, other.root);
        return *this;
    } // operator=()


    // Description: Destructor
    // Runtime: O(n)
    ~PairingPQ() {
        if(root == nullptr){
            return;
        }
        std::deque<Node*> queue;
        queue.push_back(root);
        while(queue.size() > 0){
            Node* temp = queue.front();
            if(temp->sibling != nullptr){
                queue.push_back(temp->sibling);
            }
            if(temp->child != nullptr){
                queue.push_back(temp->child);
            }
            queue.pop_front();
            delete temp;
        }
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
        if(root == nullptr){
            return;
        }
        if(root->child == nullptr){
            delete root;
            root = nullptr;
            count--;
            return;
        }
        std::deque<Node*> queue;
        Node* c = root->child;
        while(c != nullptr){
            Node* temp = c->sibling;
            c->sibling = nullptr;
            c->parent = nullptr;
            queue.push_back(c);
            c = temp;
        }
        while(queue.size() > 1){
            Node* m1 = queue.front();
            queue.pop_front();
            Node* m2 = queue.front();
            queue.pop_front();
            Node* mf = meld(m1,m2);
            queue.push_back(mf);
        }
        delete root;
        root = queue[0];
        count--;
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
        if (node == root) {
            return; //will always increase in priority so we're good
        }
        Node* temp = node->parent->child;
        if(temp == node){
            temp->parent->child = temp->sibling;
            temp->parent = nullptr;
            temp->sibling = nullptr;
            root = meld(temp, root);
            return;
        }
        while(temp->sibling != nullptr && temp->sibling != node){
            temp = temp->sibling;
        }
        temp->sibling = node->sibling;
        node->sibling = nullptr;
        node->parent = nullptr;
        root = meld(root, node);

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
    
    void copypq(const PairingPQ &pq) {
        if (pq.root == nullptr) {
            return;
        }
        std::deque<Node*> other;
        other.push_back(pq.root);
        root = nullptr;
        while (!other.empty()) {
            Node* temp = other.front();
            if(temp->child != nullptr){
                other.push_back(temp->child);
            }
            if(temp->sibling != nullptr){
                other.push_back(temp->sibling);
            }
            push(temp->elt);
            other.pop_front();
        }
    }
};

#endif // PAIRINGPQ_H
