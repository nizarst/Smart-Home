import { Injectable } from '@angular/core';
import { Database, listVal, objectVal, ref } from '@angular/fire/database';
import { Observable } from 'rxjs';

@Injectable({
  providedIn: 'root'
})
export class UsersService {
  constructor(private db: Database) { }

  getUsers() {
    const usersRef = ref(this.db, 'users/');
    return listVal(usersRef);
  }
}
