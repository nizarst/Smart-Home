import { Component, OnInit } from '@angular/core';
import { UsersService } from '../users.service'; // Adjust the path as necessary

@Component({
  selector: 'app-users-list',
  templateUrl: './users-list.component.html',
  styleUrls: ['./users-list.component.css']
})
export class UsersListComponent implements OnInit {
  // Define the attendance property to store your data
  attendance: any[] = [];

  constructor(private usersService: UsersService) { }

  ngOnInit() {
    this.usersService.getUsers().subscribe(data => {
      this.attendance = data;
    });
  }
}
