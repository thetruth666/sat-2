#ifndef __MY_VECTOR_H
#define __MY_VECTOR_H
#include <cstddef>

using namespace std;
template<class T>
class myvector
{
public:
	typedef T value_type;
	typedef value_type* pointer;
	typedef value_type* iterator;
	typedef value_type& reference;
	typedef const value_type* const_pointer;
	typedef const value_type* const_iterator;
	typedef const value_type& const_reference;
	typedef size_t size_type;
protected:
	void __allocate_and_fill(size_type n, const T& value)        //����ռ䣬������ʼֵ
	{
		iterator result = (iterator)malloc(n * sizeof(T));
		if (0 != result)
		{
			//�����ڴ�ɹ����ڵõ����ڴ��ϴ�������
			start = result;
			end_of_storage = start + n;
			finish = end_of_storage;
			while (n--)
			{
				construct(result, value);
				++result;
			}
		}
		else {
			cout << "�ڴ治�㣬������ֹ��" << endl;
			exit(0);
		}
	}
	iterator __allocate_and_copy(iterator first, iterator last, size_type n) {
		iterator result = (iterator)malloc(n * sizeof(T));
		iterator _start = result;
		if (0 != result)
		{
			while (n--) {
				construct(result, *first);
				++result;
				++first;
			}
		}
		else
		{
			exit(0);
		}
		return _start;
	}
	//��first��last������֮��[first,last)��Ԫ�ؿ�����_start��ʼ���ڴ���
	iterator __copy(iterator first, iterator last, iterator _start)
	{
		while (first < last)
		{
			*_start++ = *first++;
		}
		return _start;
	}
public:
	//������Ԫ��ָ��
	iterator begin() { return start; }
	const iterator begin() const { return start; }
	//����βԪ����һ��λ�õ�ָ��
	iterator end() { return finish; }
	const iterator end() const { return finish; }
	//�����Ĵ�С
	size_type size() const { return (size_type)(end() - begin()); }
	//������ʵ�ʴ�С
	size_type capacity() const { return (size_type)(end_of_storage - begin()); }
	//�ж������Ƿ�Ϊ��
	bool empty() { return begin() == end(); }
	//typedef ptrdiff_t difference_type;
	//Ĭ�Ϲ��캯��
	myvector() :start(0), finish(0), end_of_storage(0) { }
	//���캯������ C c(n,t):
	myvector(size_type n, const T& value) { __allocate_and_fill(n, value); }
	myvector(int n, const T& value) { __allocate_and_fill(n, value); }
	myvector(long n, const T& value) { __allocate_and_fill(n, value); }
	//���캯������ C c(n): 
	myvector(size_type n) { __allocate_and_fill(n, T()); }
	myvector(int n) { __allocate_and_fill(n, T()); }
	myvector(long n) { __allocate_and_fill(n, T()); }
	//���캯������ C c2(c1)
	myvector(const myvector<T>& mv)
	{
		start = __allocate_and_copy(mv.begin(), mv.end(), mv.end() - mv.begin());
		finish = start + (mv.end() - mv.begin());
		end_of_storage = finish;
	}
	//���캯������ C c2(b,e)
	myvector(const iterator& b, const iterator& e)
	{
		start = __allocate_and_copy(b, e, size_type(e - b + 1));
		finish = start + (e - b + 1);
		end_of_storage = finish;
	}
	//Ԫ�ز���
	//ɾ�����һ��Ԫ��
	void pop_back()
	{
		if (!empty())
		{
			--finish;
			destroy(finish);
		}
	}
	//ɾ��ָ��λ���ϵ�Ԫ��,����ָ��ɾ��Ԫ�صĵ�����
	iterator erase(iterator position)
	{
		if (position > begin() && position < end())
		{
			__copy(position + 1, finish, position);
		}
		--finish;
		destroy(finish);
		return position;
	}
	//����erase,���ݵ�������Χɾ��
	iterator erase(iterator first, iterator last)
	{
		iterator i = __copy(last, finish, first);
		destroy(i, finish);
		finish -= (last - first);
		return first;
	}
	//���ȫ��Ԫ��
	void clear()
	{
		erase(begin(), end());
	}
	//��myvector ������������һ��Ԫ��
	void push_back(const T& value)
	{
		if (finish != end_of_storage)    //������б��ÿռ�
		{
			construct(finish, value);
			++finish;
		}
		else
		{
			//��������ռ�
			const size_type old_size = size();
			const size_type new_size = (old_size == 0) ? 1 : 2 * old_size;
			iterator new_start = (iterator)malloc(new_size * sizeof(T));
			iterator new_finish = new_start;
			//�ڴ�ķ���Ҫ��ԭ���ԣ���:Ҫôȫ���ɹ���Ҫôȫ��ʧ�ܡ�
			try {
				//1.��ԭ���ݿ������µ�myvector
				//2.Ϊ�µ�Ԫ���趨��ֵx
				//3.����new_finish
				for (iterator it = begin(); it < end(); ++it)
				{
					//cout << "it:" << *it << " ";
					construct(new_finish++, *it);
				}
				construct(new_finish, value);
				++new_finish;
			}
			catch (...)
			{
				//���ʧ����
				destroy(new_start, new_finish);
				//ɾ�����뵽���ڴ�
				free(new_start);
				new_start = new_finish = NULL;
				throw;        //�׳��쳣
			}

			//�������ͷ�ԭmyvector
			destroy(begin(), end());
			//ɾ���ڴ�
			free(start);
			//������������ָ���µ�myvector
			start = new_start;
			finish = new_finish;
			end_of_storage = new_start + new_size;
		}
	}
	void insert(iterator position, size_type n, const T& value)
	{
	}
	void insert(iterator position, const T& value)
	{
		insert(position, 1, value);
	}
	//���ز�����
	reference operator[](size_type n) { return *(begin() + n); }
	const_reference operator[](size_type n) const { return *(begin() + n); }
	bool operator==(const myvector& mv)
	{
		if (mv.size() != size())
			return false;
		for (iterator it = mv.begin(); it < mv.end(); ++it)
		{
			if (*it != *(begin() + (it - mv.begin())))
				break;
		}
		if (it == mv.end())
			return true;
		else
			return false;
	}
	bool operator!=(const myvector& mv)
	{
		return !(operator==(mv));
	}
	template <class T>
	inline void destroy(T* pointer) {
		pointer->~T();                                //ֻ������һ���װ����ָ����ָ�Ķ�������---ͨ��ֱ�ӵ��������������
	}

	template <class T1, class T2>
	inline void construct(T1* p, const T2& value) {
		new (p) T1(value);                            //��placement new�� p ��ָ�Ķ����ϴ���һ������value�ǳ�ʼ�������ֵ��
	}

	template <class ForwardIterator>                //destroy�ķ����棬��������������Ϊ����
	inline void destroy(ForwardIterator first, ForwardIterator last) {
		for (; first < last; ++first)
			destroy(&*first);
	}


	inline void destroy(char*, char*) {}        //��� char * ���ػ���
	inline void destroy(wchar_t*, wchar_t*) {}    //��� wchar_t*���ػ���
private:
	iterator start;
	iterator finish;
	iterator end_of_storage;
};

#endif