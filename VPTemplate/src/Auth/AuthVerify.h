#ifndef AUTHVERIFY_H
#define AUTHVERIFY_H

/**
 * @brief  Single .auth-section function: verifies the application signature
 *         and, if correct, tears down the Authenticator and jumps into the
 *         Application's StartHandler().
 */
void verify(void);

#endif /* AUTHVERIFY_H */
