package com.byteflow.app.adapter;

import android.content.Context;
import android.graphics.Color;
import android.support.annotation.NonNull;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.RadioButton;
import android.widget.TextView;

import com.byteflow.app.R;

import java.util.List;

public class MyRecyclerViewAdapter extends RecyclerView.Adapter<MyRecyclerViewAdapter.MyViewHolder> implements View.OnClickListener {
    private List<String> mTitles;
    private Context mContext;
    private int mSelectIndex = 0;
    private OnItemClickListener mOnItemClickListener = null;

    public MyRecyclerViewAdapter(Context context, List<String> titles) {
        mContext = context;
        mTitles = titles;
    }

    public void setSelectIndex(int index) {
        mSelectIndex = index;
    }

    public int getSelectIndex() {
        return mSelectIndex;
    }

    public void addOnItemClickListener(OnItemClickListener onItemClickListener) {
        mOnItemClickListener = onItemClickListener;
    }

    @NonNull
    @Override
    public MyViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.sample_item_layout, parent, false);
        MyViewHolder myViewHolder = new MyViewHolder(view);
        view.setOnClickListener(this);
        return myViewHolder;
    }

    @Override
    public void onBindViewHolder(@NonNull MyViewHolder holder, int position) {
        holder.mTitle.setText(mTitles.get(position));
        if (position == mSelectIndex) {
            holder.mRadioButton.setChecked(true);
            holder.mTitle.setTextColor(mContext.getResources().getColor(R.color.colorAccent));
        } else {
            holder.mRadioButton.setChecked(false);
            holder.mTitle.setText(mTitles.get(position));
            holder.mTitle.setTextColor(Color.GRAY);
        }
        holder.itemView.setTag(position);
    }

    @Override
    public int getItemCount() {
        return mTitles.size();
    }

    @Override
    public void onClick(View v) {
        if (mOnItemClickListener != null) {
            mOnItemClickListener.onItemClick(v, (Integer) v.getTag());
        }
    }

    public interface OnItemClickListener {
        void onItemClick(View view, int position);
    }

    class MyViewHolder extends RecyclerView.ViewHolder {
        RadioButton mRadioButton;
        TextView mTitle;

        public MyViewHolder(View itemView) {
            super(itemView);
            mRadioButton = itemView.findViewById(R.id.radio_btn);
            mTitle = itemView.findViewById(R.id.item_title);
        }
    }
}