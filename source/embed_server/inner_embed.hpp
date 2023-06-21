#include<vector>
#include<random>

class BaseInnerEmbedding{
public:
    BaseInnerEmbedding(int _embed_size, int _embed_dim): embed_size_(_embed_size), embed_dim_(_embed_dim){} 
    virtual void query(const std::vector<int>& input, std::vector<std::vector<float>>& output) = 0;
protected:
    int embed_size_;
    int embed_dim_;
};

class RandomInnerEmbedding : public BaseInnerEmbedding{
public:
    RandomInnerEmbedding(int _embed_size, int _embed_dim);
    void query(const std::vector<int>& input, std::vector<std::vector<float>>& output) override;
    inline float generateRandom() {return distr_(random_engine_);}
private:
    std::default_random_engine random_engine_;
    std::uniform_real_distribution<float> distr_;
};