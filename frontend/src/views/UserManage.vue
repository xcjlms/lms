<template>
  <div class="user-manage">
    <h2>用户管理</h2>
    <table class="user-table">
      <thead>
        <tr>
          <th>ID</th>
          <th>用户名</th>
          <th>角色</th>
          <th>状态</th>
          <th>操作</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="u in users" :key="u.user_id">
          <td>{{ u.user_id }}</td>
          <td>{{ u.username }}</td>
          <td>
            <select v-model="u.role" @change="updateUser(u)">
              <option value="READER">读者</option>
              <option value="MANAGER">管理员</option>
            </select>
          </td>
          <td>
            <select v-model="u.status" @change="updateUser(u)">
              <option value="ACTIVE">正常</option>
              <option value="SUSPENDED">停用</option>
            </select>
          </td>
          <td>
            <button @click="updateUser(u)" :disabled="u.saving">保存</button>
            <span v-if="u.saving">保存中…</span>
          </td>
        </tr>
        <tr v-if="users.length === 0"><td colspan="5">暂无用户</td></tr>
      </tbody>
    </table>
  </div>
</template>

<script setup>
import { ref, onMounted } from 'vue'
import * as api from '../api/index.js'

const users = ref([])

async function fetchUsers() {
  try {
    const res = await api.getUsers()
    users.value = (res.data?.users || []).map(u => ({ ...u, saving: false }))
  } catch (e) {
    alert('加载用户失败：' + e.message)
  }
}

async function updateUser(user) {
  if (user.saving) return
  user.saving = true
  try {
    await api.updateUser(user.user_id, user.role, user.status)
  } catch (e) {
    alert('更新失败：' + e.message)
  } finally {
    user.saving = false
  }
}

onMounted(fetchUsers)
</script>

<style scoped>
.user-manage { padding: 20px; }
.user-table { width: 100%; border-collapse: collapse; }
.user-table th, .user-table td { border: 1px solid #ebeef5; padding: 10px; text-align: left; }
.user-table th { background: #f5f7fa; }
.user-table select { padding: 4px 8px; border-radius: 4px; border: 1px solid #dcdfe6; }
</style>
