#include <iostream>
#include <exception>
using namespace std;

// Implemented struct, inherited from the exception STL to throw error of unaccessible memory
struct null_ptr_exception : public std::exception {
    const char* what() const noexcept override {
        return "Attempting to access a null pointer";
    }
};

template <typename T>
class smart_ptr {
public:
    // Initializes empty constructor; should not throw
    smart_ptr() noexcept :
        ptr_(nullptr),
        ref_(new int(0)) {}

    /* Creates a constructor where smart_ptr initialized to
    raw_ptr and increment its ref count by 1; should not throw */
    explicit smart_ptr(T*& raw_ptr) noexcept :
        ptr_(raw_ptr),
        ref_(new int(1)) {}

    /* Create a constructor to where smart_ptr is initialized to
    raw_ptr and increment its ref count by 1; raw_ptr is then
    set to nullptr; should not throw */
    explicit smart_ptr(T*&& raw_ptr) noexcept :
        ptr_(raw_ptr),
        ref_(new int(1)) {
        raw_ptr = nullptr;
    }

    /* Copy constructs a pointer and ref count from rhs. Its ref count
    increments by 1; should not throw */
    smart_ptr(const smart_ptr& rhs) noexcept :
        ptr_(rhs.ptr_),
        ref_(rhs.ref_) {
        if (ptr_)
            (*ref_)++;
    }

    /* Move constructer to pass values to pointer from rhs;
    should not throw */
    smart_ptr(smart_ptr&& rhs) noexcept :
        ptr_(rhs.ptr_),
        ref_(rhs.ref_) {
        rhs.ptr_ = nullptr;
        rhs.ref_ = nullptr;
    }

    /* Makes shallow copy of rhs' data. If data is not equal to rhs'
    data, release() is called and copy is initiated. Ref count is incremented;
    should not throw */
    smart_ptr& operator=(const smart_ptr& rhs) noexcept {
        if (this != &rhs) {
            release();
            ptr_ = rhs.ptr_;
            ref_ = rhs.ref_;
            if (ptr_)
                (*ref_)++;
        }
        return *this;
    }

    /* Should steal the r value's pointer data and move it to
    the smart pointer. Should not throw */
    smart_ptr& operator=(smart_ptr&& rhs) noexcept {
        cout << "Move assignment operator called\n";
        if (this != &rhs) {
            release();
            ptr_ = rhs.ptr_;
            ref_ = rhs.ref_;
            rhs.ptr_ = nullptr;
            rhs.ref_ = nullptr;
        }
        return *this;
    }

    /* Will not do anything if smart pointer is nullptr or ref count
    of 1; return false. Otherwise, make deep copy of object is created,
    ref count is decremented, and new copy will be object that smart pointer
    points to. Strong guarantee employed for minimal throwing chance */
    bool clone() {
        if (!ptr_ || *ref_ == 1)
            return false;
        T* new_ptr = new T(*ptr_);
        --(*ref_);
        ptr_ = new_ptr;
        ref_ = new int(1);
        return true;
    }

    // Returns ref count. Should not throw
    int ref_count() const noexcept {
        return *ref_;
    }

    /* Dereference operator returns a reference to the referred object.
    Will throw if attempting to access a nullptr */
    T& operator*() const {
        if (!ptr_) throw null_ptr_exception();
        return *ptr_;
    }

    /* Arrow operator returns a pointer. Will throw if attempting
    to access a nullptr */
    T* operator->() const {
        if (!ptr_) throw null_ptr_exception();
        return ptr_;
    }

    // Implemented release() function to mimic smart pointer's release()
    // Will deallocate all dynamic memory
    ~smart_ptr() {
        release();
    }

private:
    T* ptr_;               // pointer to the referred object
    int* ref_;             // pointer to a reference count

    /* Created a private release function to mimic a smart pointer
    release function. Returns value and deallocates all memory */
    void release() {
        if (ref_ && --(*ref_) == 0) {
            delete ptr_;
            delete ref_;
        }
        ptr_ = nullptr;
        ref_ = nullptr;
    }
};

struct Point { int x = 2; int y = -5; };

int main() {
    int* p{ new int { 42 } };
    smart_ptr<int> sp1{ p };

    cout << "Ref count is " << sp1.ref_count() << endl;    // Ref Count is 1
    {
        smart_ptr<int> sp2{ sp1 };
        cout << "Ref count is " << sp1.ref_count() << endl;    // Ref Count is 2
        cout << "Ref count is " << sp2.ref_count() << endl;    // Ref Count is 2
    }


    cout << "Ref count is " << sp1.ref_count() << endl;    // Ref Count is 1

    smart_ptr<int> sp3;

    cout << "Ref count is " << sp3.ref_count() << endl;    // Ref Count is 0

    sp3 = sp1;

    cout << "Ref count is " << sp1.ref_count() << endl;    // Ref Count is 2
    cout << "Ref count is " << sp3.ref_count() << endl;    // Ref Count is 2

    smart_ptr<int> sp4{ std::move(sp1) };

    cout << *sp4 << " " << *sp3 << endl;        // prints 42 42
    //cout << *sp1 << endl;                       // throws null_ptr_exception

    smart_ptr<Point> sp{ new Point };
    cout << sp->x << " " << sp->y << endl;   // prints 2  -5

    smart_ptr<double> dsp1{ new double {3.14} };
    smart_ptr<double> dsp2, dsp3;

    dsp3 = dsp2 = dsp1;

    cout << dsp1.ref_count() << " " << dsp2.ref_count() << " " << dsp3.ref_count() << endl;
    // prints 3 3 3
    cout << *dsp1 << " " << *dsp2 << " " << *dsp3 << endl;
    // prints 3.14 3.14 3.14

    dsp1.clone();        // returns true

    cout << dsp1.ref_count() << " " << dsp2.ref_count() << " " << dsp3.ref_count() << endl;
    // prints 1 2 2
    cout << *dsp1 << " " << *dsp2 << " " << *dsp3 << endl;
    // prints 3.14 3.14 3.14

  // Implemented extra test for move assignment operator
    smart_ptr<int> sp5;
    sp5 = std::move(sp3); // Should trigger the move assignment operator
    cout << *sp5;

}
